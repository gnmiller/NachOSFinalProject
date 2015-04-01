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
	Create( "someFile" );
	OpenFileId fd = Open( "someFile" );
	Write( "this is a string!", 32, fd );
	Close( fd );
	Exit( 0 );
}
