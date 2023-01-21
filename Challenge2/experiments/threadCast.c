#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>

void main(){

  int x = 2, y;

  pthread_t threadID = (pthread_t) x;

  printf("threadID = %ld\n", threadID);

  y = (int) threadID;

  printf("y = %d\n", y);
  
}
