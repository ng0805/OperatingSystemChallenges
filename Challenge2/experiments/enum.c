#include <stdio.h>
#include <stdlib.h>

enum thread_status
  {
   TS_EXITED,
   TS_RUNNING,
   TS_READY
  };


void main(){ 
 enum  thread_status ts = TS_EXITED;

  if(ts == TS_EXITED){

    printf("works\n");
  }
}
