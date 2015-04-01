// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <string.h>

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
	
#include "fileIO.syscall"



/**
 * Create a process in SC_Exec
**/
void createProcess(int arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
}

/**
 * Execute a file
**/
int 
Exec_Syscall_Func( )
{
	char *fileName = currentThread->space->read(machine->ReadRegister(4), 0); //get exe name from register 4
	
	int args = machine->ReadRegister(5);//get argument from register 5
	Thread* thread = new Thread("user", currentThread);
	DEBUG('t', "\nExe File Name: %s\n", fileName);
	SpaceId sid = -1;
	try
	{
		thread->space = new AddrSpace( fileName );
		sid = thread->getID();
		thread->Fork(&createProcess, 0);
		
		DEBUG('t', "Successfully create EXE %s with ID %d\n", fileName, sid);
	}
	catch (int e)
	{
		DEBUG('t', "Failed to create address space for EXE %s\n", fileName);
		printf("Failed to create address space for EXE %s\n", fileName);
		delete thread;
		return -1;
	}
	
	/*
	machine->WriteRegister(2, (int)sid);
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc += 4;
	machine->WriteRegister(NextPCReg, pc);
	*/
	
	return sid;	
}

/**
 * Exit current executable
**/
void 
Exit_Syscall_Func()
{
	int status = machine->ReadRegister(4);
	currentThread->notifyParent(status);//doesn't matter what the value of status is
	//currentThread->Yield();
	currentThread->Finish();
}

/**
 * Join the childprintf("fuckerEXEC\n");
**/
void 
Join_Syscall_Func()
{
	int cid = machine->ReadRegister(4);//child thread id
	machine->WriteRegister(2, -1);
	if(cid >=0 && currentThread->child != NULL)
	{
		ChildThread* childRecord = currentThread->child;
		do
		{
			if(childRecord->id == cid)//find this child
			{
				DEBUG('t', "Waiting on child thread %d\n", cid);
				childRecord->join_sem->P();
				machine->WriteRegister(2, childRecord->status);
				break;
			}
			childRecord = childRecord->next;
		} while(childRecord != currentThread->child);
	}
	// remove update to PC here, ExceptionHandler should do this
}

void
ExceptionHandler(ExceptionType which)
{
	DEBUG( 's', "SYSCALL: %d\n", which );
    int type = machine->ReadRegister(2);
	int sys_ret = 0; // store return if one is expected
				// will get written to r2 at the end of business
	if( which == SyscallException )
	{
		/* debug flag s for syscall? */
		if( type == SC_Halt )
		{
			DEBUG( 's', "Shutdown, initiated by user program.\n" );
			interrupt->Halt();
		}
		else if( type == SC_Create )
		{
			DEBUG( 's', "Create, initiated by user program.\n" );
			Create_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Open )
		{
			DEBUG( 's', "Open, initiated by user program.\n" );
			sys_ret = Open_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Write )
		{
			DEBUG( 's', "Write, initiated by user program.\n" );
			sys_ret = Write_Syscall_Func( machine->ReadRegister(4),
										  machine->ReadRegister(5), machine->ReadRegister(6) );
		}
		else if( type == SC_Read )
		{
			DEBUG( 's', "Read, initiated by user program.\n" );
			sys_ret = Read_Syscall_Func( machine->ReadRegister(4),
										  machine->ReadRegister(5), machine->ReadRegister(6) );
		}
		else if( type == SC_Close )
		{
			DEBUG( 's', "Close, initiated by user program.\n" );
			Close_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Exit )
		{
			DEBUG( 's', "Exit, initiated by user program.\n" );
			printf("Exiting in SC_EXIT\n");
			Exit_Syscall_Func();
		}
		else if( type == SC_Exec )
		{
			DEBUG( 's', "Exec, initiated by user program.\n" );
			sys_ret = Exec_Syscall_Func( );
		}
		else if( type == SC_Join )
		{
			DEBUG( 's', "Join, initiated by user program.\n" );
			Join_Syscall_Func();
		}
		else 
		{
			printf( "Unexpected user mode exception %d %d.\n", which, type );
			ASSERT(FALSE);
		}
		/* clean up procedure */
		machine->WriteRegister( 2, sys_ret ); // write the return code, if 0 still not
											//important since the caller wont care anyway
		/* increment the PC now */
		machine->WriteRegister( PrevPCReg, machine->ReadRegister( PCReg ) );
		machine->WriteRegister( PCReg, machine->ReadRegister( NextPCReg ) );
		machine->WriteRegister( NextPCReg, machine->ReadRegister( PCReg ) + 4 );
		return;
	}
	else if( which == PageFaultException)
	{
		int badAddr = machine->ReadRegister(BadVAddrReg);
		DEBUG('t', "PageFault on Address: 0x%x, on Page: %d\n", badAddr, badAddr / PageSize);
		if(currentThread->space == NULL)
		{
			DEBUG('t', "Pagefault on thread with no address space.\n");
            currentThread->notifyParent(-1);
            currentThread->Finish();
		}
		if(currentThread->space->load_page(badAddr/PageSize) == -1)
		{
			DEBUG('t', "Cannot load virtual page: %d\n", badAddr/PageSize);
			currentThread->notifyParent(-1);
            currentThread->Finish();
		}
	}
	else if ( which != SyscallException )
	{
		//printf( "Unsupported exception type: %d!\n", which );
		return;
	}
}
