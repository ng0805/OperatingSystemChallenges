#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "ec440threads.h"

/* Extracted from private libc headers. These are not part of the public
 * interface for jmp_buf.
 */
#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC 7

static jmp_buf buf1,buf2;


void
foo()
{
  setjmp(buf2);

  printf("stack ptr 2: %ld\n", ptr_demangle(buf2->__jmpbuf[JB_RSP]));
}

int
main()
{
  setjmp(buf1);
  // hey lets play with this...
  /* buf->__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)start_thunk);
  buf->__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)topStackPtr);
  buf->__jmpbuf[JB_R12] = (unsigned long int)start_routine;

  longjmp(buf, 1);*/

  printf("stack ptr 1: %ld\n", ptr_demangle(buf1->__jmpbuf[JB_RSP]));

  int i = 6;
  
  setjmp(buf2);

  printf("stack ptr 2: %ld\n", ptr_demangle(buf2->__jmpbuf[JB_RSP]));
  
  // foo();
  
  printf("After long jump \n");
  return 0;
}
