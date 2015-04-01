/* fromcons.c
 *
 *	Simple program to read characters from the
 *      console and echo them. Stop when a Q is read.
 *      Assumes uniprogrammed system.
 *	
 */

#include "syscall.h"

int
main()
{
  int count=0;
  char c;

  while (1) {
    Read(&c, 1, ConsoleInput);
    if ( c == 'Q' ) {
      prints("\n", ConsoleOutput);
      printd(count, ConsoleOutput);
      prints(" characters seen.\n", ConsoleOutput);
      Halt();
    }
    else {
      count++;
      Write(&c, 1, ConsoleOutput);
    }
  }

    /* not reached */
}

/* Print a null-terminated string "s" on open file
   descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  while (*s != '\0') {
    Write(s,1,file);
    s++;
  }
}

/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i;
  char c;

  if (n < 0) {
    Write("-",1,file);
    n = -n;
  }
  if ((i = n/10) != 0)
    printd(i,file);
  c = (char) (n % 10) + '0';
  Write(&c,1,file);
}
