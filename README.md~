# nachos-miller-li-spring15
| Author | Can Li | Greg Miller |
---------|--------|-------------|
| DATE UPDATED| -- | 2/23/15 |
| SLIP DAYS REMAINING | 4 | 4 |
| SLIP DAYS USED | 0 | 0 |

## Bridge Question
1. Our bridge solution does not guarantee fairness or safety from starvation
	but does attempt to hold some fairness, in that there is some bound
	to how many times a process group might be waiting.
	
	2. When a car arrives it checks if it may enter the bridge (traffic
		is the correct direction and space). It then notifies other
		cars in the same direction that if they're waiting they may go
		up to the maximum load. 
			
			* If a car enters the entry protocol while there is traffic
				in the current direction (d0) while another	car is
				waiting in the opposite direction (d1) then it is not 
				deterministic in our solution which car may go.

			* If all the cars currently on the bridge exit before cars
				travelling in d0 can get scheduled then the last car off
				changes the direction to the opposite, but this is not
				enough to make the decision. If cars in d0 are scheduled
				next then they may go as the bridge is empty, if cars in
				d1 schedule next they may go as CUR_DIR = 1.


## Answers to question 4
1. What causes time to advance in NachOS?
	1. Three things may cause time to advance:
		1. interrupts are turned on or off
		2. a user program instruction is executed
		3. when nothing is on the ready queue.
2. What code is executed along with Fork() when a thread executes?
	2. The thread allocates itself some space on the stack, then disables
		interrupts and assigns intself to the ready queue. By doing this
		first we take up some memory, ensuring there's enough for this
		process. Then disable interrupt to prevent another process from
		jumping in and creating trouble while we try to queue up. If 
		interrupts were not disabled it's possible that some other process could
		jump on the CPU and create race conditions revolving around the scheduler.
3. i386 Context Switch
	1. To perform a context-swicth, all registers in the old thread has to be 
		saved. After that, new values would be loaded into registers from the
		context block of the next thread. In Nachos, the routine SWITCH would
		complete this work.
	2. Switching CPU from one thread to another causes the context switch. 
		Specifically, When invoking methods Thread::Yield and Thread::Sleep, 
		if other thread is waiting, then context-switch would occur. 
	3. The state would be stored in the thread context block. In
		Nachos impelementation, Thread::stackTop and Thread::machineState
		store stack pointer and other registers respectively.
