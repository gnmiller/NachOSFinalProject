#ifndef NACHOS_CAR_CPP
#define NACHOS_CAR_CPP
#include "car.h"

Car::Car() : Thread( "uninit car" )
{
	dir = -1; // set dir to indicate that this is not a real car
};

/**
 * Car::Car()
 * 
 * Construct a new car with direction i, named debugname
 * cond should point to a shared condition variable, that is shared
 * 	among all cars with the same direction.
 * 	
 * lock, similar to cond, should point to a mutex that is shared
 * 	among ALL the cars.
 */
Car::Car( int i, char *debugname ) : Thread( debugname )
{
	if ( i < 0 || i > 1 )
	{
		dir = -1;
	}
	dir = i;
};

/** run this thread */
void Car::Fork( VoidFunctionPtr func, int arg )
{
	Thread::Fork( func, arg );
};

#endif

