/**
 * A container class used by addrspace objects.
 * Contains the information about open files that
 * a process has.
 *
 * See addrspace.cc for implementation.
 */

#ifndef FD_LIST_H
#define FD_LIST_H

#include "bitmap.h"

class Lock;
class FD_List
{
	BitMap map;
	void **list;
	Lock *lock;
	int size;
public:
	FD_List();
	FD_List( int );
	~FD_List();
	void *fd_get( int );
	int fd_put( void* );
	void *fd_remove( int );
};

#endif