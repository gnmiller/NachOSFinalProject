#ifndef CHILDTHREAD_H
#define CHILDTHREAD_H
#ifdef USER_PROGRAM

#include "thread.h"
#include "synch.h"

class ChildThread {
public:
    ChildThread(Thread* t);
    ~ChildThread();

    Thread* thread;
    int status;
    int id;
    Semaphore* join_sem;
    ChildThread* next;
    ChildThread* prev;
};

#endif //USER_PROGRAM
#endif //CHILDTHREAD_H
