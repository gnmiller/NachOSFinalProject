#include "syscall.h"
prints(char *s, OpenFileId file)
{
  while( *s != '\0' )
  {
	Write( s, 1, file );
	++s;
  }
}
int main(){
	Create( "testFile" );
	OpenFileId fd = Open( "testFile" );
	Write( "this is a string!", 32, fd );
	Exit( 0 );
}
