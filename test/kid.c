/* kid.c
 *
 * Child in the fork/kid system.
 *
 */

#include "syscall.h"

int
main()
{
  int i, j;
 
  Write("Kid exists\n",11,ConsoleOutput);
  for (i=0; i<10000; i++) j++ ;
  /* loop to delay kid initially; hope parent gets to Join and sleeps */
  Exit(17);
  /* Should not get past here */
  Write("KID after Exit()!\n", 18, ConsoleOutput);
  Halt();
    /* not reached */
}
