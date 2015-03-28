#include "syscall.h"
#include <string.h>

int main()
{
  OpenFileId fd;
  create( "newFile" );
  printf("Created \"newFile\"\n");
  fd = open( "newFile" );
  printf("Opened \"newFile\"\n");
  char *string = "Test string 1";
  Write( string, strlen( string ), fd );
  printf("Wrote to \"newFile\"\n");
  char *string1 = malloc( sizeof( char ) * 32 );
  int readSize = Read( string1, sizeof( char ) * 32, fd );
  printf( "Read string (%d): %s from \"newFile\"\n" );
  
  Exit( 0 );
}