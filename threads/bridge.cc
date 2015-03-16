#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "car.h"
#include "car.cpp"
#include <list>
#define MAX_LOAD 3

void OneVehicle( int );
void ArriveBridge( int );
void CrossBridge( int );
void ExitBridge( int );

Lock *Car_Lock;
Condition *CV_DIR0; // blocks one dir
Condition *CV_DIR1; // blocks the other dir
Lock *CV_DIR1_LOCK; // lcck for the cv
Lock *CV_DIR0_LOCK; // lock for other cv
int DIR0_COUNT; // counts cars on the bridge
int DIR1_COUNT; // counts cars on the bridge (the other way)
int CUR_DIR; // indicates which direction traffic is going
char DEBUG_FLAG = 't';
Lock *test_lock;
void ThreadFunc( int i );
void ThreadTest()
{
	
	void (*THREAD_RUN)(int) = &ThreadFunc;
	std::list<Car*> cars0;
	std::list<Car*> cars1;
	Car_Lock = new Lock( "car_lock" );
	CV_DIR0_LOCK = new Lock( "cvd0lock" );
	CV_DIR1_LOCK = new Lock( "cvd1lock" );
	CV_DIR0 = new Condition( "cvd0" );
	CV_DIR1 = new Condition( "cvd1" );
	CUR_DIR = 0;
	DIR0_COUNT = 0;
	DIR1_COUNT = 0;
	for( int i = 0; i < 10; ++i )
	{
		char name[32];
		sprintf( name, "name%d", i );
		Car *test_car = new Car( 0, name );
		cars0.push_front(test_car);
		Car *test_car2 = new Car( 1, name );
		cars1.push_front(test_car2);
		
	}
	
	for( std::list<Car*>::iterator iter = cars0.begin(); iter != cars0.end(); ++iter )
	{
		(*iter)->Fork(THREAD_RUN, 0);
	}
	for( std::list<Car*>::iterator iter = cars1.begin(); iter != cars1.end(); ++iter )
	{
		(*iter)->Fork(THREAD_RUN, 1);
	}
	//test_lock = new Lock("lock_name");
	
}

void ThreadFunc( int i )
{
	OneVehicle(i);
}

/**
 * A car arrives at the bridge, attempts to cross it
 * and reports on when it is crossing via DEBUG()
 * 
 * dir indicates which direction the car is going, which
 * must be a binary value (0 or 1). The car object has this
 * property and we check to ensure no problems.
 */
void OneVehicle( int dir )
{
	ArriveBridge( dir );
	CrossBridge( dir );
	ExitBridge( dir );
};

void ArriveBridge( int dir )
{
	printf( "A car travelling in direction %d has arrived.\n", dir );
	if( dir == 0 )
	{
		DIR0_CHECK:
		Car_Lock->Acquire();
		/* if there are no cars at all or there are cars going
			in our direction */
		if( CUR_DIR == 0  || ( DIR0_COUNT == 0 && DIR1_COUNT == 0 ) )
		{
			CUR_DIR = 0; // in case we went bc no traffic
			
			/* if there are 0-2 cars on the bridge */
			//printf( "car d: %d\ncount:%d\nc_dir:%d\n" );
			if( DIR0_COUNT < MAX_LOAD )
			{
				++DIR0_COUNT;
				CV_DIR0_LOCK->Acquire();
				for( int j = DIR0_COUNT; j < MAX_LOAD; ++j )
				{
					CV_DIR0->Signal( CV_DIR0_LOCK );
				}
				CV_DIR0_LOCK->Release();
				Car_Lock->Release();
			}
			else /* no room get in the queue */
			{
				CV_DIR0_LOCK->Acquire();
				Car_Lock->Release();	
				CV_DIR0->Wait( CV_DIR0_LOCK );
				CV_DIR0_LOCK->Release();
				goto DIR0_CHECK;
			}
		}
		else /* traffic going the other way */
		{
			CV_DIR0_LOCK->Acquire();
			Car_Lock->Release();	
			CV_DIR0->Wait( CV_DIR0_LOCK );
			CV_DIR0_LOCK->Release();
			goto DIR0_CHECK;
		}
	}
	else if( dir == 1 )
	{
		DIR1_CHECK:
		Car_Lock->Acquire();
		if( CUR_DIR == 1  || ( DIR0_COUNT == 0 && DIR1_COUNT == 0 ) )
		{
			CUR_DIR = 1; // in case we went bc no traffic
			
			/* if there are 0-2 cars on the bridge */
			//printf( "car d: %d\ncount:%d\nc_dir:%d\n" );
			if( DIR1_COUNT < MAX_LOAD )
			{
				++DIR1_COUNT;
				CV_DIR1_LOCK->Acquire();
				for( int j = DIR1_COUNT; j < MAX_LOAD; ++j )
				{
					CV_DIR1->Signal( CV_DIR1_LOCK );
				}
				CV_DIR1_LOCK->Release();
				Car_Lock->Release();
			}
			else /* no room get in the queue */
			{
				CV_DIR1_LOCK->Acquire();
				Car_Lock->Release();	
				CV_DIR1->Wait( CV_DIR0_LOCK );
				CV_DIR1_LOCK->Release();
				goto DIR1_CHECK;
			}
		}
		else /* traffic going the other way */
		{
			CV_DIR1_LOCK->Acquire();
			Car_Lock->Release();	
			CV_DIR1->Wait( CV_DIR1_LOCK );
			CV_DIR1_LOCK->Release();
			goto DIR1_CHECK;
		}
	}
	else
	{
		printf("error!\n"); 
	}
};

void CrossBridge( int dir )
{
	printf( "A car travelling in direction %d is now crossing.\n", dir );
};

void ExitBridge( int dir )
{
	printf( "A car travelling in direction %d is now exiting.\n", dir );
	if( dir == 0 )
	{
		Car_Lock->Acquire();
		--DIR0_COUNT; // get off the bridge
		if( DIR0_COUNT == 0 )
		{
			CUR_DIR = 1; // give the other guys a chance
			CV_DIR1_LOCK->Acquire();
			CV_DIR1->Signal( CV_DIR1_LOCK ); // this guy will let more guys go
			CV_DIR1_LOCK->Release();
			Car_Lock->Release();
		}
		else
		{
			Car_Lock->Release();
		}
	}
	else if( dir == 1 )
	{
		Car_Lock->Acquire();
		--DIR1_COUNT;
		if( DIR1_COUNT == 0 )
		{
			CUR_DIR = 0; // swap sides
			CV_DIR0_LOCK->Acquire();
			CV_DIR0->Signal( CV_DIR0_LOCK );
			CV_DIR0_LOCK->Release();
			Car_Lock->Release();
		}
		else
		{
			Car_Lock->Release();
		}
	}
	else
	{
		printf("error!\n");
	}
};
	