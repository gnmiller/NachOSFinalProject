/* fork.c
 *
 * Parent in simple parent/child system.
 *
 */

#include "syscall.h"

int
main()
{

  SpaceId child1 = 0, child2 = 0;
  int joinval;
  char *args[2];

  prints("PARENT exists\n", ConsoleOutput);
  args[0] = "kid";
  args[1] = (char *)0;  
  child1 = Exec("./test/child1", args);
  child2 = Exec("./test/child2", args);
  prints("PARENT after exec; child1 pid is ", ConsoleOutput);
  printd((int)child1, ConsoleOutput);
  prints("\n", ConsoleOutput);
  
  prints("PARENT is about to Join child1\n", ConsoleOutput);
  joinval = Join(child1);
  prints("PARENT off Join with value of ", ConsoleOutput);
  printd(joinval, ConsoleOutput);
  prints("\n", ConsoleOutput);
  
  prints("PARENT after exec; child2 pid is ", 32, ConsoleOutput);
  printd((int)child2, ConsoleOutput);
  prints("\n", ConsoleOutput);
  
  prints("PARENT is about to Join child2\n", ConsoleOutput);
  joinval = Join(child2);
  prints("PARENT off Join with value of ", ConsoleOutput);
  printd(joinval, ConsoleOutput);
  prints("\n", ConsoleOutput);

  prints("PARENT is about to Join non-exist child\n", ConsoleOutput);
  joinval = Join(3);
  prints("PARENT off Join with value of ", ConsoleOutput);
  printd(joinval, ConsoleOutput);
  prints("\n", ConsoleOutput);	

  Halt();
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
