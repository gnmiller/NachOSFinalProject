#ifndef NACHOS_CAR_PROTO_CPP
#define NACHOS_CAR_PROTO_CPP

#include "synch.h"

#define MAX_LOAD 3
#define DEBUG_FLAG 't'
extern Lock *Car_Lock;
extern Condition *CV_DIR0; // blocks one dir
extern Condition *CV_DIR1; // blocks the other dir
extern Lock *CV_DIR1_LOCK; // lcck for the cv
extern Lock *CV_DIR0_LOCK; // lock for other cv
extern int DIR0_COUNT; // counts cars on the bridge
extern int DIR1_COUNT; // counts cars on the bridge (the other way)
extern int CUR_DIR; // indicates which direction traffic is going
extern void (*THREAD_RUN)(int); // func pointer for cars to use when the run

void OneVehicle( int );
int ArriveBridge( int );
int CrossBridge( int );
int ExitBridge( int );

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
	if( dir < 0 || dir > 1 )
	{
		return;
	}
	else
	{
		if( ArriveBridge( dir ) != 0 )
		{
			return;
		}
		CrossBridge( dir ); // should not be able to fail assuming dir is 0 or 1
		if( ExitBridge( dir ) != 0 )
		{
			return;
		}
		return;
	}
};

/**
 * A car arrives at the bridge.
 * 
 * dir: which direction is it travelling (0 or 1)
 * 
 * A car, when it arrives, checks which direction the traffic is travelling,
 * if it travelling in the correct direction OR there are exactly 0 cars
 * travelling in either direction it attempts to cross. If the direction
 * does not match and there are cars crossing it blocks.
 * 
 * Once it has confirmed it can try to cross it tests how many cars
 * are on the bridge, if it is less than 3 it can cross, it also
 * notifies a number of cars proportional to the free space
 * (ex if there is now 1 car, the current car crossing, it signals
 * two times) to the condition that matches this direction to allow
 * more cars to cross.
 * 
 * NOTE: This method can lead to starvation of the opposite side!
 * 
 * When a car has been block and then awoken it returns to the initial check,
 * of is the direction matching or are there exactly 0 cars on the bridge and
 * then blocks/continues accordingly.
 */
int ArriveBridge( int dir )
{
	DEBUG( DEBUG_FLAG, "A car has arrived going in direction: %d\n", dir );
	if( dir == 0 ) // which way am i going?
	{
		/* we check if the direction is ours and THEN if we can get on OR
			if there are even any cars on the bridge, if there are no cars
			we may cross immediately */
		DIR0_CHECK:
		if( CUR_DIR == 0 || ( DIR0_COUNT == 0 && DIR1_COUNT == 1 ) )
		{
			CUR_DIR = 0; // in case there were 0 cars crossing
			Car_Lock->Acquire(); // lock it up
			if( DIR0_COUNT < MAX_LOAD ) // space for us on the bridge
			{
				++DIR0_COUNT;
				/* let some cars go, it does not matter if they are waiting or not 
					we hold the lock and continue execution (MESA) if someone is
					waiting they are awoken eventually to go and will recheck the cv */
				for( int i = DIR0_COUNT; i < MAX_LOAD; ++i ) CV_DIR1->Signal( CV_DIR1_LOCK );
				Car_Lock->Release();
				return 0;
			}
			else // no room we have to wait
			{
				Car_Lock->Release();
				CV_DIR0->Wait( CV_DIR0_LOCK );
				/* *gasp!* a GOTO to recheck the condition when someone wakes us */
				goto DIR0_CHECK;
			}
		}
		else // not going our way so we wait
		{
			Car_Lock->Release();
			CV_DIR0->Wait( CV_DIR0_LOCK );
			/* *gasp!* a GOTO to recheck the condition when someone wakes us */
			goto DIR0_CHECK; 
		}
	}
	else if( dir == 1 ) // this is a mirror of the above code save checking against DIR1
	{
		/* we check if the direction is ours and THEN if we can get on OR
			if there are even any cars on the bridge, if there are no cars
			we may cross immediately */
		DIR1_CHECK:
		if( CUR_DIR == 1 || ( DIR0_COUNT == 0 && DIR1_COUNT == 1 ) )
		{
			CUR_DIR = 1; // in case no one was crossing
			Car_Lock->Acquire(); // lock it up
			if( DIR1_COUNT < MAX_LOAD ) // space for us on the bridge
			{
				++DIR1_COUNT;
				/* let some cars go, it does not matter if they are waiting or not 
					we hold the lock and continue execution (MESA) if someone is
					waiting they are awoken eventually to go and will recheck the cv */
				for( int i = DIR1_COUNT; i < MAX_LOAD; ++i ) CV_DIR1->Signal( CV_DIR1_LOCK );
				Car_Lock->Release();
				return 0;
			}
			else // no room we have to wait
			{
				Car_Lock->Release();
				CV_DIR1->Wait( CV_DIR1_LOCK );
				/* *gasp!* a GOTO to recheck the condition when someone wakes us */
				goto DIR1_CHECK;
			}
		}
		else // not going our way so we wait
		{
			Car_Lock->Release();
			CV_DIR1->Wait( CV_DIR1_LOCK );
			/* *gasp!* a GOTO to recheck the condition when someone wakes us */
			goto DIR1_CHECK; 
		}
	}
	else
	{
		return -1;
	}
};

/**
 * Print a simple message of when a car is crossing
 * eg has returned from ArriveBridge()
 */
int CrossBridge( int dir )
{
	DEBUG( DEBUG_FLAG, "A car is crossing the bridge in direction: %d\n", dir );
	return 0;
};

/**
 * A car has finished crossing and runs the exit
 * procedure
 * 
 * dir: the direction of crossing.
 *
 * Once a car has returned from both arrive and cross it
 * reduces the counter corresponding to its direction.
 * If this reduces the counter to 0, eg there are now exactly
 * 0 cars on the bridge it will change the CUR_DIR to the opposite
 * side and then signal the opposite condition variable.
 * 
 * This method tries to retain some measure of fairness and reduce
 * the possibility of starvation. Though it is possible that one
 * direction is still starved.
 */
int ExitBridge( int dir )
{
	DEBUG( DEBUG_FLAG, "A car has exited going in direction: %d\n", dir );
	if( dir == 0 )
	{
		Car_Lock->Acquire();
		--DIR0_COUNT;
		if( DIR0_COUNT == 0 ) // there are no more cars crossing currently
		{
			CUR_DIR = 1; // change the dir so other people might go
			/* it is important to NOTE: if no cars are waiting the system does not
				deadlock due to this signal being lost due to the way that
				ArriveBridge() is structured. */
			CV_DIR1->Signal( CV_DIR1_LOCK );
			/* it also important to NOTE: that we can and do release the lock NOW
			 instead of before the signal, as this is Mesa we know that we keep
			 the CPU over the guy we just woke up */
			Car_Lock->Release();
			return 0;
		}
		else
		{
			Car_Lock->Release();
			return 0;
		}
		return -1;
	}
	else if( dir == 1 ) // this a mirror of above
	{
		Car_Lock->Acquire();
		--DIR1_COUNT;
		if( DIR1_COUNT == 0 ) // there are no more cars crossing currently
		{
			CUR_DIR = 0; // change the dir so other people might go
			/* it is important to NOTE: if no cars are waiting the system does not
				deadlock due to this signal being lost due to the way that
				ArriveBridge() is structured. */
			CV_DIR0->Signal( CV_DIR0_LOCK );
			/* it also important to NOTE: that we can and do release the lock NOW
			 instead of before the signal, as this is Mesa we know that we keep
			 the CPU over the guy we just woke up */
			Car_Lock->Release();
			return 0;
		}
		else
		{
			Car_Lock->Release();
			return 0;
		}
		return -1;
	}
	else
	{
		// error
		return -1;
	}	
};

void (*THREAD_RUN)(int) = &OneVehicle; // func pointer defined for threads to use

#endif
