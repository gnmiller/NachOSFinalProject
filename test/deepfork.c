/* deepfork.c
 *
 * Parent execs/joins kid who execs/joins kid.
 *
 */

#include "syscall.h"

int
main()
{

  SpaceId kid;
  int joinval;
  char *args[2];

  Write("PARENT exists\n", 14, ConsoleOutput);
  args[0] = "kid4";
  args[1] = (char *)0;
  kid = Exec("./test/kid4",args);
  Write("PARENT after exec; kid pid is ", 30, ConsoleOutput);
  printd((int)kid, ConsoleOutput);
  Write("\n", 1, ConsoleOutput);

  Write("PARENT about to Join kid\n", 25, ConsoleOutput);
  joinval = Join(kid);
  Write("PARENT off Join with value of ", 30, ConsoleOutput);
  printd(joinval, ConsoleOutput);
  Write("\n", 1, ConsoleOutput);

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
