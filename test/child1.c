/* child1.c
 *
 * Child from multiprog.
 *
 */

#include "syscall.h"

int
main()
{
  int i, j;
 
  Write("Child1 exists\n",11,ConsoleOutput);
  for (i=0; i<10000; i++) j++ ;
  /* loop to delay kid initially; hope parent gets to Join and sleeps */
  Write("Child1 is about to exit\n", 21, ConsoleOutput);
  Exit(0);
  /* Should not get past here */
  Write("Child1 after Exit()!\n", 18, ConsoleOutput);
  Halt();
    /* not reached */
}
