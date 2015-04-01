/* kid5.c
 *
 * Second-level child in the deepfork system.
 *
 */

#include "syscall.h"

int
main()
{
  int i, j;
  Write("Kid5 exists\n",12,ConsoleOutput);
  for (i=0; i<10000; i++) j++;
  /* loop to delay kid initially */
  Exit(5);
  /* Should not get past here */
  Write("KID5 after Exit()!\n", 19, ConsoleOutput);
  Halt();
    /* not reached */
}
