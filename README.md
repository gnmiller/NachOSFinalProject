# nachos-miller-li-spring15
# Project B
| Author | Can Li | Greg Miller |
---------|--------|-------------|
| DATE SUBMITTED | -- | 4/3/15 |
| SLIP DAYS REMAINING | 1 | 1 |
| SLIP DAYS USED | 3 | 3 |

## Summary
1. System calls:
	1). void Halt();
	-- Stop Nachos, and print out performance stats

	2). void Exit(int status);
	-- Finish the current user process

	3). SpaceId Exec(char* name, char* args[]);
	-- Run the executable, stored in the Nahos file "name", "args" 
		is the remaining arguments passed into it. If failed, return -1. 

	4). int Join(SpaceId id);
	-- Current thread joins its child thread specified by "id". If 
		successful, return 0. If the "id" refers to a non-exist thread 
		or non-child thread, return -1. 

	5). void Create(char* name)
	-- Create a file at location "name" on the NachOS FS. Does not matter
		if it exists already or not. It is created in RW mode on UNIX by
		the NachOS stub.
	
	6). OpenFiledId Open(char* name);
	-- Open the file at name for reading and writing. Is tested for
		permissions if it does not reside in the working directory.
		Upon succes the file is opened for RW and the OpenFileId returned.
		Upon failure -1 is returned.

	7). void Write(char* buffer, int size, OpenFileId id);
	-- Write size bytes from the buffer into the OpenFileId returned by
		calling Open(). If a size of <=0 is specified the function returns
		immediately and no data is written. If an invalid id is specified
		(not opened, < 0, or stdin) the call returns. As the return is void there
		is no return value, however similar to UNIX write() the number of
		written bytes is available to the system.

	8). int Read(char* buffer, int size, OpenFileId id);
	-- Read size bytes into buffer from the OpenFileId returned by
		calling Open(). If passed a size <= 0 the call returns 0 (or -1 for
		< 0 ) immediately. On success the number of bytes read is returned.
		On failure -1 is returned. Additionally: The call disallows 
		reading from stdout.

	9). void Close(OpenFileId id);
	-- Close an OpenFileId returned by calling Open(). If it called
		on an invalid FD the function returns (UNIX close() not called).

2. Exceptions:
	1). SyscallException -- indicate system call occurs, find corresponding system calls handle function
	2). OverflowException -- indicate overflow occurs, notify parent thread that current thread is about to exit and then finish this thread
	3). AddressErrorException -- indicate absurd reference of address occurs, print out segmentation fault message, notify parent thread that current thread is about to exit and then finish this thread
	
3. Schedule algorithm: Round Robin
Very easy to implement, just raise a interception between a fixed time period

4. SynchConsole:
Use "./usrprog/nachos -sc" to enter the synchronization console test mode

# Test cases
## File IO System Calls
The file IO system call test suite contains the following tests:
	Note: All test cases for this suite utilize the directory: dir_test/
	to store their results. It is necessary to guarantee correctness that
	this directory is empty before execution.
	1. The first test is what might be expected of a typical user who is 
		not doing something foolish.
		
		The test creates, opens, writes, closes, opens and reads from the file 
		newFile. After the completion of the read operation the data
		is printed to stdout.
	2. The second case attempts to operate on a file (does.not.exist) 
		that does not exist. Attempts are made to Open, Read and Write to
		the file.
	3. The next case tests opening a file twice (alreadyOpen) with two seperate
		fds and writing data to each of them. The expected result given the NachOS FS
		is that the output will be "an string" (the content of the *second*
		call to write).
	4. The fourth test case attempts to create the same file two times (alreadyExists).
	5. The following test attempts to open, close, reopen and write to the same
		file (oco_test). A write is attempted on the old fd after it is closed. The expected
		output in this test should be the content of only the write to the second
		fd opened.
	6. This test attempts to close a file that is not open.
	7. This test writes with a size of -1. The file after the call should 
		remain empty (rwtest).
	8. This test reads with an invalid size (-1) (rwtest2). The expected output should be nothing.
	9. This test attempts to create and open a file that the user does not have
		sufficient permissions on the underlying system for. The file is: /no.permissions
		This test depends on the user not being able to access the directory /.
	10. The final test in this suite attempts to read from an empty file (emptyFile).
		The expected output should be nothing.

## Process Management System Calls

## Dazzle
	* Our implementation does not present any particularly "dazzling" features
		beyond what was required.
	* One showing is that the system does detect if the user has access to a path
		when opening files. This applies only to the UNIX FS however and prevents
		the OS crashing when insufficient permissiosn are present.
	

