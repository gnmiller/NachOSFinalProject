#ifndef NACHOS_CAR_H
#define NACHOS_CAR_H
#include "thread.h"
/**
 * FILE: car.h
 * Prototype for the car thread.
 */
class Car : public Thread {
	public:
		Car();
		Car( int, char* ); // we will just use the default priority
		int dir;
		void Fork( VoidFunctionPtr func, int arg );
};

#endif
