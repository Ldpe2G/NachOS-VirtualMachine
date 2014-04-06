/* add.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{
  int result;
  
  result = Add(42, 23);
  Write("Add result:\n ",13,1);
  Putw(result,1);
  Write("\n",1,1);
  Halt();
  /* not reached */
}
