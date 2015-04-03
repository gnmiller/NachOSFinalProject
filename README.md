# nachos-miller-li-spring15
# Project B
| Author | Can Li | Greg Miller |
---------|--------|-------------|
| DATE UPDATED| -- | 4/03/15 |
| SLIP DAYS REMAINING | 0 | 0 |
| SLIP DAYS USED | 4 | 4 |

(a) Document 
1. System calls:
	1). void Halt();
	-- Stop Nachos, and print out performance stats

	2). void Exit(int status);
	-- Finish the current user process

	3). SpaceId Exec(char* name, char* args[]);
	-- Run the executable, stored in the Nahos file "name", "args" is the remaining arguments passed into it. If failed, return -1. 

	4). int Join(SpaceId id);
	-- Current thread joins its child thread specified by "id". If successful, return 0. If the "id" refers to a non-exist thread or non-child thread, return -1. 

	5). void Create(char* name)

	6). OpenFiledId Open(char* name);

	7). void Write(char* buffer, int size, OpenFileId id);

	8). int Read(char* buffer, int size, OpenFileId id);

	9). void Close(OpenFileId id);

2. Exceptions:
	1). SyscallException -- indicate system call occurs, find corresponding system calls handle function
	2). OverflowException -- indicate overflow occurs, notify parent thread that current thread is about to exit and then finish this thread
	3). AddressErrorException -- indicate absurd reference of address occurs, print out segmentation fault message, notify parent thread that current thread is about to exit and then finish this thread
	
3. Schedule algorithm: Round Robin
Very easy to implement, just raise a interception between a fixed time period

4. SynchConsole:
Use "./usrprog/nachos -sc" to enter the synchronization console test mode

