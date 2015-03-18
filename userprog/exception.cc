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
	
/** 
 * Create a new file on the fs for the user with the name stored at
 * the virtual address specified in addr that is size bytes long.
 * 
 * Returns 0 on success.
 */
void
Create_Syscall_Func( unsigned int addr, int size )
{	
	char *buf = new char[ size + 1 ]; // strlen (size) + \0
	/* error  check */
	if( buf == NULL ) { printf("Failed to alloc buffer in create_syscall\n.");
	if( readVMem( addr, size, buf ) == -1 ) // read the filename from mem
	{
		printf("Failed reading VMem in SysCall.\n" );
		delete buf;
		return;
	}
	
	/* null byte.. */
	buf[ size ] = '\0';
	
	/* tell the FS to make the file */
	fileSystem->Create( buf, 0 );
	DEBUG( 's', "Created file: %s requested by userprog\n" );
	
	delete[] buf;
	return;
} // create_syscall_func

/**
 * Open a file specified by the name pointed to by the address addr that is 
 * size bytes long. If successful the file is stored into the executing processes
 * adress space's file table and the ID is returned for later reference.
 * 
 * The NachOS FS presented does not appear to have support (at least a way to 
 * interface with the FS in a way that does) for file permissions currently.
 * We have assumed that all files will inherently have the required permissions
 * when it is attempted to open() it.
 * 
 * On an error -1 is returned.
 */
int
Open_Syscall_Func( unsigned int addr, int size )
{
	char *buf = new char[ size + 1 ];
	OpenFile *file;
	int fd;
	
	/* error  check */
	if( buf == NULL ) { printf("Failed to alloc buffer in open_syscall\n.");
	if( readVMem( addr, size, buf ) == -1 ) // read the filename from mem
	{
		printf("Failed to read VMem in SysCall.\n");
		delete[] buf;
		return -1;
	}
	 /* null byte.. */
	buf[ size ] = '\0';
	
	/* tell the FS to open it up */
	file = fileSystem->Open( buf );
	
	
	if( file )
	{
		/* put it on the threads table */
		if( ( fd = currentThread->space->open_files.fd_put( file )) == -1 )
			delete file;
		DEBUG( 's', "Opened file: %s requested by userprog\n" );
		return fd;
	}
	delete[] buf;
	else return -1;
} // open_syscall_func

/**
 * Accessing the memory at location virt_addr, reading the data to be written
 * Copy it into a buffer then write the contents of that buffer to the file 
 * specified in ID; if ID is stdin (ConsoleInput in NachOS) it is an error.
 *
 * Returns -1 on error and the amount of characters written otherwise.
 */
int
Write_Syscall_Func( unsigned int virt_addr, int size, int fd )
{
	char *buf;
	OpenFile *file;
	buf = new char[size];
	
	/* stdin = error */
	if( id == ConsoleInput ) return -1;
	
	/* error  check */
	if( buf == NULL ) { printf("Failed to alloc buffer in write_syscall\n."); };
	if( readVMem( virt_addr, size, buf ) == -1 ) // read the data from mem
	{
		printf("Failed to read from memory in write_syscall\n");
		delete[] buf;
		return -1;
	}
	
	/* output to console .. 
		NOTE: may need to be updated when/if SynchConsole is implemented */
	if( fd == ConsoleOutput )
	{
		for( int i = 0; i < size; ++i )
		{
			printf("%c", buf[i] ); // write to stdout directly
		}
		delete[] buf;
		return i;
	}
	/* write to the file specified if not stdout */
	else
	{
		/* check if we have the fd open */
		f = currentThread->space->open_files->fd_get( id )
		if( f > 1 ) // no stderror just in (0) out (1)...
		{
			int temp = f->Write( buf, size );			
			delete[] buf;
			return temp;
		}
		else
		{
			printf( "Failed to write to file in write syscall (bad id)" );
			delete[] buf;
			return -1;
		}
	}
} // write_syscall_func

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
			DEBUG('s', "Shutdown, initiated by user program.\n" );
			interrupt->Halt();
		}
		else if( type == SC_Create )
		{
			DEBUG('s', "Create, initiated by user program.\n" );
			Create_Syscall_Func( machine->ReadRegister(4), machine->ReadRegister(5) );
		}
		else if( type == SC_Open )
		{
			DEBUG('s', "Open, initiated by user program.\n" );
			sys_ret = Open_Syscall_Func( machine->ReadRegister(4), machine->ReadRegister(5) );
		}
		else if( type == SC_Write )
		{
			DEBUG('s', "Write, initiated by user program.\n" );
			sys_ret = Write_Syscall_Func( machine->ReadRegister(4), machine->ReadRegister(5) );
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
