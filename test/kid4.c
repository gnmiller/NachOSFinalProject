/* kid4.c
 *
 * First-level child in the deepfork system.
 *
 */

#include "syscall.h"

int
main()
{
  int i, joinval, j;
  SpaceId kid = -1;
  char *args[2];
  Write("KID4 exists\n",12,ConsoleOutput);	
  for (i=0; i++; i<100000) j++;
  /* loop to delay kid initially */	
  args[0] = "kid5";
  args[1] = (char *)0;
  kid = Exec("./test/kid5", args);
  Write("KID4 after exec; kid5 pid is ", 29, ConsoleOutput);
  printd((int)kid, ConsoleOutput);
  Write("\n", 1, ConsoleOutput);

  Write("KID4 is about to Join kid5\n", 27, ConsoleOutput);
  joinval = Join(kid);
  Write("KID4 off Join with value of ", 28, ConsoleOutput);
  printd(joinval, ConsoleOutput);
  Write("\n", 1, ConsoleOutput);
  Write("KID4 is about to exit\n", 22, ConsoleOutput);	
  Exit(4);
  /* Should not get past here */
  Write("KID4 after Exit()!\n", 19, ConsoleOutput);
  Halt();
    /* not reached */
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
