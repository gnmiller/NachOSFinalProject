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

/**
 * Translate a virtual addr to physical and read size bytes
 * into buf from the address specified
 * 
 * return -1 on an error
 */
int
readVMem( unsigned int virt_addr, int size, char *buf )
{
	bool res;
	int readB = 0; // bytes we read
	int *phys_addr = new int; // physical address
	
	while( readB >= 0 && readB < size )
	{
		/* read the data from mem */
		res = machine->ReadMem( virt_addr, 1, phys_addr );
		/* store it to the buffer */
		buf[readB++] = *phys_addr;		
		if( res != 0 ) return -1;
		
		/* increment to next byte */
		virt_addr++;
	}
	
	delete phys_addr;
	return readB;
}

/**
 * Translate a virtual addr to physical and write size bytes
 * from buf into the address specified
 * 
 * return -1 on an error
 */
int
writeVMem( unsigned int virt_addr, int size, char *buf )
{
	bool res;
	int writeB = 0;
	
	while( writeB >= 0 && writeB < size )
	{
		res = machine->WriteMem( virt_addr, 1, (int)( buf[ writeB++ ] ) );
		if( res != 0 ) return -1;
		
		/* increment */
		virt_addr++;
	}
	
	return writeB;
}
	
#include "fileIO.syscall"

void
ExceptionHandler(ExceptionType which)
{
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
			Create_Syscall_Func( machine->ReadRegister(4), machine->ReadRegister(5) );
		}
		else if( type == SC_Open )
		{
			DEBUG( 's', "Open, initiated by user program.\n" );
			sys_ret = Open_Syscall_Func( machine->ReadRegister(4), machine->ReadRegister(5) );
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
			// Close_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Exit )
		{
			DEBUG( 's', "Exit, initiated by user program.\n" );
			// Exit_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Fork )
		{
			DEBUG( 's', "Fork, initiated by user program.\n" );
			// Fork_Syscall_Func( machine->ReadRegister(4) );
		}
		else if( type == SC_Exec )
		{
			DEBUG( 's', "Exec, initiated by user program.\n" );
			// sys_ret = Exec_Syscall_Func( machine-ReadRegister(4), machine->ReadRegister(5) );
		}
		else if( type == SC_Join )
		{
			DEBUG( 's', "Join, initiated by user program.\n" );
			// Join_Syscall_Func( machine-ReadRegister(4) );
		}
		else if( type == SC_Yield )
		{
			DEBUG( 's', "Yield, initiated by user program.\n" );
			// Yield_Syscall_Func( machine-ReadRegister(4) );
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
	else if ( which != SyscallException )
	{
		printf( "Unsupported exception type!\n" );
		return;
	}
}
