#include "syscall.h"
/**
 * test case suite for file io system calls
 * (open, close, read and write)
 *
 * The following program tests various things that
 * the average user may do during execution of his or her
 * code.
 * 
 * The cases presented are (in order of test):
 * 	- Typical operation (all correct usage):
 * 		* Create, Open, Write then Read the file dir_test/newFile
 *	- 
 */
prints(char *s, OpenFileId file)
{
  while( *s != '\0' )
  {
	Write( s, 1, file );
	++s;
  }
}
int main()
{
	char buffer[128];
	char buffer1[128];
	char buffer2[128];
	char nl[81];
	int i;
	for( i = 0; i < 80; ++i ) nl[i] = '*';
	nl[81] = '\0';
	
	int read = 0;
	prints("FILE IO TEST CASES\n", ConsoleOutput );
	prints( nl, ConsoleOutput );
	
	/* undefined ref to memset ??? */
	buffer[0] = 'a'; buffer[1] = 'n'; buffer[2] = ' '; 
	buffer[3] = 's'; buffer[4] = 't'; buffer[5] = 'r'; 
	buffer[6] = 'i'; buffer[7] = 'n'; buffer[8] = 'g'; 
	buffer[10] = '\0';
	
	buffer2[0] = 'a'; buffer2[1] = 'n'; buffer2[2] = ' '; 
	buffer2[3] = 's'; buffer2[4] = 't'; buffer2[5] = 'r'; 
	buffer2[6] = 'i'; buffer2[7] = 'n'; buffer2[8] = 'g'; 
	buffer2[10] = '2'; buffer2[11] = '\0';

	for( i = 0; i < 128; ++i ) buffer1[i] = 0;
	/* normal operation */
	prints( "\n\nStandard procedure: Create, Open, Write, Close, Read, Close\n", ConsoleOutput );	
	Create( "dir_test/newFile" );
	OpenFileId fd = Open( "dir_test/newFile" );
	Write( buffer, 32, fd );
	Close( fd );
	fd = Open( "dir_test/newFile" );
	read = Read( buffer1, 32, fd );
	Close( fd );
	prints( "Read Data: ", ConsoleOutput );
	prints( buffer1, ConsoleOutput );
	
	/* open a file that doesnt exist */
	prints( "\n\nFile does not exist: Open, Write, Read\n", ConsoleOutput );
	
	OpenFileId nullfd = Open( "dir_test/does.not.exist" );
	Write( buffer, 32, nullfd );
	for( i = 0; i < 128; ++i ) buffer1[i] = 0;
	Read( buffer1, 32, nullfd );
	Close( nullfd ); // should print a diagnostic message when it fails...
	prints( buffer, ConsoleOutput );
	
	/* open a file already open */
	prints( "\n\nFile is already open: Open, Open, Write/Read, Write/Read (each fd)\n", ConsoleOutput );
	Create( "dir_test/alreadyOpen" );
	OpenFileId alreadyOpen = Open( "dir_test/alreadyOpen" );
	OpenFileId alreadyOpen2 = Open( "dir_test/alreadyOpen" );	
	Write( buffer, 32, alreadyOpen );	
	Write( buffer2, 32, alreadyOpen2 );	
	Close( alreadyOpen );
	Close( alreadyOpen2 );
	
	/* create an existing file */
	prints( "\n\nFile already exists: Create, Create\n", ConsoleOutput );
	Create( "dir_test/alreadyExists" );
	Create( "dir_test/alreadyExists" );
	
	/* open close open */
	prints( "\n\nOpen, close, open on same file\n", ConsoleOutput );
	
	Create( "dir_test/oco_test" );
	OpenFileId oco_test = Open( "dir_test/oco_test" );
	Close( oco_test );
	OpenFileId oco_test2 = Open( "dir_test/oco_test" );
	Write( buffer, 32, oco_test ); // error no data writes
	Write( buffer, 32, oco_test2 ); // should write
	Close( oco_test2 );
	
	/* close a file not opened */
	prints( "\n\nClose a file not opened\n", ConsoleOutput );
	OpenFileId notopen;
	Close( notopen ); // uninitialized
	notopen = -1;
	Close( notopen ); // initialized to bad value

	/* write bad size */
	prints( "\n\nInvalid write size\n", ConsoleOutput );
	
	Create( "dir_test/rwtest" );
	OpenFileId rwtest = Open( "dir_test/rwtest" );
	Write( buffer, -1, rwtest );
	Close( rwtest );
	// rwtest should be empty
	
	/* read bad size */
	prints( "\n\nInvalid read size\n", ConsoleOutput );
	
	Create( "dir_test/rwtest2" );
	rwtest = Open( "dir_test/rwtest2" );
	for( i = 0; i < 128; ++i ) buffer1[i] = 0;
	Read( buffer1, -1, rwtest );
	Close( rwtest );
	prints( buffer1, ConsoleOutput );	
	// rwtest2 should also be empty as should buffer1
	
	/* insufficient permissions */
	prints( "\n\nCreate/Open file without permissions\nNote: This test relies on no access to \"/\"\n", ConsoleOutput );
	Create( "/no.permissions" );
	Open( "/no.permissions" );
	
	/* read empty file */
	prints( "\n\nRead from file with no content\n", ConsoleOutput );
	Create( "dir_test/emptyFile" );
	fd = Open( "dir_test/emptyFile" );
	for( i = 0; i < 128; ++i ) buffer1[i] = 0;
	Read( buffer1, 128, fd );
	prints( "Read: ", ConsoleOutput );
	prints( buffer1, ConsoleOutput );
	
	Exit( 0 );
}
