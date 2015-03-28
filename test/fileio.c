#include "syscall.h"
#define nl "********************************************************************************\n"
int main()
{
	char buffer[128];
	char buffer1[128];
	char buffer2[128];
	int read = 0;
	Write( "FILE IO TEST CASES\n", 32, ConsoleOutput );
	Write( nl, 81, ConsoleOutput );
	/* undefined ref to memset ??? */
	buffer[0] = 'a'; buffer[1] = 'n'; buffer[2] = ' '; 
	buffer[3] = 's'; buffer[4] = 't'; buffer[5] = 'r'; 
	buffer[6] = 'i'; buffer[7] = 'n'; buffer[8] = 'g'; 
	buffer[10] = '\0';
	
	buffer2[0] = 'a'; buffer2[1] = 'n'; buffer2[2] = ' '; 
	buffer2[3] = 's'; buffer2[4] = 't'; buffer2[5] = 'r'; 
	buffer2[6] = 'i'; buffer2[7] = 'n'; buffer2[8] = 'g'; 
	buffer2[10] = '2'; buffer2[11] = '\0';

	/* normal operation */
	Write( nl, 81, ConsoleOutput );
	Write( "Standard procedure: Create, Open, Write, Read, Close\n", 128, ConsoleOutput );
	
	Create( "newFile" );
	OpenFileId fd = Open( "newFile" );
	Write( buffer, 32, fd );
	read = Read( buffer1, 32, fd );
	Close( fd );
	Write( buffer, 32, ConsoleOutput );
	
	/* open a file that doesnt exist */
	Write( nl, 81, ConsoleOutput );
	Write( "File does not exist: Open, Write, Read\n", 128, ConsoleOutput );
	
	OpenFileId nullfd = Open( "does.not.exist" );
	Write( buffer, 32, nullfd );
	Read( buffer, 32, nullfd );
	Close( nullfd ); // should print a diagnostic message when it fails...
	Write( buffer, 32, ConsoleOutput );
	
	/* open a file already open */
	Write( nl, 81, ConsoleOutput );
	Write( "File is already open: Open, Open, Write/Read, Write/Read (each fd)\n", 128, ConsoleOutput );
			   
	Create( "alreadyOpen" );
	OpenFileId alreadyOpen = Open( "alreadyOpen" );
	OpenFileId alreadyOpen2 = Open( "alreadyOpen" );
	
	Write( buffer, 32, alreadyOpen );
	read = Read( buffer1, 128, alreadyOpen2 );
	Write( buffer1, 128, ConsoleOutput );
	
	Write( buffer2, 32, alreadyOpen2 );
	read = Read( buffer1, 128, alreadyOpen2 );
	Write( buffer1, 128, ConsoleOutput);
	
	Close( alreadyOpen );
	Close( alreadyOpen2 );
	
	/* create an existing file */
	Write( nl, 81, ConsoleOutput );
	Write( "File already exists: Create, Create\n", 128, ConsoleOutput );
	
	Create( "alreadyExists" );
	Create( "alreadyExists" );
	
	/* open close open */
	Write( nl, 81, ConsoleOutput );
	Write( "Open, close, open on same file\n", 128, ConsoleOutput );
	
	Create( "oco_test" );
	OpenFileId oco_test = Open( "oco_test" );
	Close( oco_test );
	OpenFileId oco_test2 = Open( "oco_test" );
	Write( buffer, 32, oco_test ); // error no data writes
	Write( buffer, 32, oco_test2 ); // should write
	Close( oco_test2 );
	
	/* close a file not opened */
	Write( nl, 81, ConsoleOutput );
	Write( "Close a file not opened", 128, ConsoleOutput );
	
	OpenFileId notopen;
	Close( notopen ); // uninitialized
	notopen = -1;
	Close( notopen ); // initialized to bad value
	
	/* read an empty file */
	Write( nl, 81, ConsoleOutput );
	Write( "Read an empty file\n", 128, ConsoleOutput );
	
	Create( "emptyFile" );
	OpenFileId emp = Open( "emptyFile" );
	read = Read( buffer1, 128, emp );
	
	/* write bad size */
	Write( nl, 81, ConsoleOutput );
	Write( "Invalid write size\n", 128, ConsoleOutput );
	
	Create( "rwtest" );
	OpenFileId rwtest = Open( "rwtest" );
	Write( buffer, -1, rwtest );
	Close( rwtest );
	
	/* read bad size */
	Write( nl, 81, ConsoleOutput );
	Write( "Invalid read size\n", 128, ConsoleOutput );
	
	Create( "rwtest2" );
	rwtest = Open( "rwtest2" );
	Read( buffer, -1, rwtest );
	Close( rwtest );
	
	Exit( 0 );
}
