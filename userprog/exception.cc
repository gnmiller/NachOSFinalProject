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
#include <libgen.h>
#include <unistd.h>

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
 * Create a new file on the fs for the user with the name stored at
 * the virtual address specified in addr that is size bytes long.
 * 
 * Returns 0 on success.
 */
void
Create_Syscall_Func( unsigned int addr )
{	
	char *buf = currentThread->space->read( addr, 0 );
	if( buf == NULL )
	{ 
		DEBUG( 'f', "Failed to alloc buffer in create_syscall\n." ); 
		return;
	}
	
	/* test if we can create the specified file */
	char tmp[256];
	strcpy( tmp, buf );
	dirname( tmp );
	if( access( tmp, R_OK|W_OK ) != 0 ) // NachOS does not differentiate so we need to check both
	{
		DEBUG( 'f', "Insufficient permissions to file: %s\n", buf );
		return;
	}
	
	/* tell the FS to make the file */
	fileSystem->Create( buf, 0 );
	DEBUG( 'f', "Created file: %s requested by userprog\n", buf );

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
Open_Syscall_Func( unsigned int addr )
{
	OpenFile *file;
	int fd;
	
	/* error  check */
	char *buf = currentThread->space->read( addr, 0 );
	if( buf == NULL )
	{ 
		DEBUG( 'f', "Failed to alloc buffer in open_syscall\n"); 
		return -1;
	}
	
	/* test if we can open the specific file */
	char tmp[256];
	strcpy( tmp, buf );
	dirname( tmp );
	if( access( tmp, R_OK|W_OK ) != 0 ) // NachOS does not differentiate so we need to check both
	{
		DEBUG( 'f', "Insufficient permissions to file: %s\n", buf );
		return -1;
	}
	
	/* tell the FS to open it up */
	file = fileSystem->Open( buf );
	
	if( file )
	{
		/* put it on the threads table */
		if( ( fd = currentThread->space->open_files.fd_put( file )) == -1 )
			delete file;
		if( fd == 0 ) return -1;
		DEBUG( 'f', "Opened file: %s requested by userprog\n", buf );
		return fd;
	}
	else
	{
		return -1;
	}
	
	
} // open_syscall_func

/**
 * Accessing the memory at location virt_addr, reading the data to be written
 * Copy it into a buffer then write the contents of that buffer to the file 
 * specified in ID; if ID is stdin (ConsoleInput in NachOS) it is an error.
 *
 * Returns -1 on error and the amount of characters written otherwise.
 */
int
Write_Syscall_Func( unsigned int addr, int size, int fd )
{
	
	OpenFile *file;
	
	/* invalid fd */
	if( fd < 0 )
	{
		return -1;
	}
	
	/* error check */
	if( size == 0 ) return 0;
	if( size < 0 ) return -1;
	
	/* stdin = error */
	if( fd == ConsoleInput )
	{
		DEBUG( 'f', "You cannot write to stdin!\n" );
		return -1;
	}
	
	/* error  check */
	char *buf = currentThread->space->read( addr, size );
	if( buf == NULL )
	{
		DEBUG( 'f', "Failed to alloc buffer in write_syscall.\n"); 
		return -1;
	}
	
	/* output to console .. 
		NOTE: may need to be updated when/if SynchConsole is implemented */
	if( fd == ConsoleOutput )
	{
		int count;
		for( count = 0; count < size; ++count )
		{
			printf("%c", buf[ count ] ); // write to stdout directly
		}
		return count;
	}
	/* write to the file specified if not stdout */
	else
	{
		/* check if we have the fd open */
		file = (OpenFile*) currentThread->space->open_files.fd_get( fd );
		if( file ) // not null, etc
		{
			int temp = file->Write( buf, size );			
			return temp;
		}
		else
		{
			DEBUG( 'f', "Failed to write to file in write syscall (bad id).\n" );
			return -1;
		}
	}
} // write_syscall_func

/**
 * Read data into the buffer pointed at by virt_addr from the file
 * specified by fd (at most size bytes).
 * 
 * The function will check and return an error if the file is not open.
 * 
 * Return -1 on error and the bytes read on success.
 */
int
Read_Syscall_Func( unsigned int addr, int size, int fd )
{
	/* error check */
	if( size == 0 ) return 0;
	if( size < 0 ) return -1;
	if( fd < 0 ) return -1;
	
	char *buf;
	OpenFile *file;
	buf = new char[ size ];
	
	/* checking errors */
	if( fd == ConsoleOutput )
	{
		DEBUG( 'f', "You cannot read from stdout!\n" );
		return -1;
	}
	if( buf == NULL )
	{
		DEBUG( 'f', "Error allocating buffer in read syscall.\n" );
	}
	
	/* read from stdin..
		NOTE: may require modification when SynchConsole comes into play */
	if( fd == ConsoleInput )
	{
		scanf( "%s", buf );
		
		/* copy into memory for user */
		currentThread->space->write( addr, buf, size );
		/* we could have written less than size ... */
		int str_len = strlen( buf );
		delete[] buf;
		return str_len-1;
	}
	/* read from file */
	else
	{
		file = (OpenFile *) currentThread->space->open_files.fd_get( fd );
		if( file )
		{
			int read_size = file->Read( buf, size );
			if( read_size > 0 )
			{
				currentThread->space->write( addr, buf, size );
				delete[] buf;
				return read_size;
			}
			else if( read_size == 0 )
			{
				// NOTE: should be removed before production?
				DEBUG( 'f', "Did not read any data on read().\n" );
				delete[] buf;
				return read_size;
			}
		}
		else
		{
			DEBUG( 'f', "Bad id, failed to read from file.\n" );
			delete[] buf;
			return -1;
		}
	}
	return -1; // this should be an error if we get here
}

/**
 * On success closes the FD specified, on failure does nothing.
 * There is no error correction, if the file isnt open nothing happens;
 * in any case there is no return value.
 */
void
Close_Syscall_Func( int fd )
{
	OpenFile *file = (OpenFile *)NULL;
	
	if( fd < 0 )
	{
		return;
	}
	
	file = (OpenFile *)currentThread->space->open_files.fd_remove( fd );
	
	if( file )
	{
		delete file;
	}
	else
	{
		DEBUG( 'f', "Tried to close a file that has not been opened by the current process.\n" );
	}
} // close syscall


/**
 * Create a process in SC_Exec
 */
void createProcess(int arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
}

/**
 * Execute a file
 */
int 
Exec_Syscall_Func( )
{
	char *fileName = currentThread->space->read(machine->ReadRegister(4), 0); //get exe name from register 4
	int args = machine->ReadRegister(5);//get argument from register 5
	printf("currentThread: %s\n", currentThread->getName());
	Thread* thread = new Thread("user", currentThread);
	DEBUG('t', "\nExe File Name: %s\n", fileName);
	SpaceId sid = -1;
	try
	{
		thread->space = new AddrSpace( fileName );
		if(args != 0)
			thread->space->createStackArgs(args, fileName);
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
 */
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
 */
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
				childRecord->join_sem->P();//would be V() in destructor of child Thread
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
	else if (which == AddressErrorException)
	{
		IntStatus i = interrupt->SetLevel(IntOff);
		printf("Segmentation fault\n");
		currentThread->notifyParent(-1);
		currentThread->Finish();
		//ASSERT(FALSE);
		interrupt->SetLevel(i);
		
	}
	else if(which == OverflowException)
	{
		IntStatus i = interrupt->SetLevel(IntOff);
		printf("Overflow fault\n");
		currentThread->notifyParent(-1);
		currentThread->Finish();
		//ASSERT(FALSE);
		interrupt->SetLevel(i);
	}
	else if(which == ReadOnlyException)
	{
		int badAddr =  machine->ReadRegister(BadVAddrReg);
		if(currentThread->space->allow_writes(badAddr/PageSize) == -1)
		{
			currentThread->notifyParent(-1);
			currentThread->Finish();
		}
	}
	else
	{
		printf( "Unsupported exception type: %d!\n", which );
		return;
	}
}
