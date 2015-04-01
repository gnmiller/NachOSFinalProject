/*---------------------------------------------------------
 * hellofile.c
 *---------------------------------------------------------
 *   Simple program to put string in a newly Created file
 *   named hello.out.
 *
 *   Expect creation/truncation of file "hello.out".
 *   Expect return value of 2 from first call to Open()
 *   Expect "hello.out" to contain "Hello, file.\n" 
 *       when execution terminates.
 *--------------------------------------------------------
 */

#include "syscall.h"

int
main()
{

    char *s = "Hello, file.\n";

    OpenFileId myfile;

    Create("hello.out");
    Write("Past the Create()\n", 18, ConsoleOutput);
    myfile = Open("hello.out");
    Write("Past the Open(): returned ", 26, ConsoleOutput);
    printd((int)myfile, ConsoleOutput);
    Write("\n", 1, ConsoleOutput);
    
    Write( s, 13, myfile);
    Write("Past the Write()\n", 17, ConsoleOutput);
    /* Will do an implicit close. */
    Halt();
    /* not reached */
}

/* Print an integer "n" on open file descriptor "file". */

printd(n, file)
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
