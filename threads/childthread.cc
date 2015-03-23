#ifdef USER_PROGRAM

#include "childthread.h"
//----------------------------------------------------------------------
// ChildThread::ChildThread
//      Create an element in the linked list of children of this thread. The
//      argument 'thread' is the new thread being inserted.
//
//      The linked list is doubly linked, and circular.
//----------------------------------------------------------------------
ChildThread::ChildThread(Thread* t) {
    status = -1;
    join_sem = new Semaphore("ChildThread", 0);
    thread = t;

    next = prev = this;
}

//----------------------------------------------------------------------
// ChildThread::~ChildThread
//      Remove this thread from the parent's record.
//----------------------------------------------------------------------
ChildThread::~ChildThread() {
    delete join_sem;
}
#endif //USERPROG
