#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void main(){

  pthread_mutex_t *mutex;

  pthread_t threadList[10];
  
  threadList[0] = (pthread_t) 56;

  mutex = malloc(sizeof(pthread_mutex_t));

  mutex -> __data.__owner = 10;
  
  printf("owner: %d\n", mutex -> __data.__owner);

  mutex -> __data.__lock = 0;

  printf("next thread = %ld\n", threadList[mutex -> __data.__lock]);


  free(mutex);
  
  
  pthread_barrier_t *barrier;

  barrier = malloc(sizeof(*barrier));

  barrier -> __align = (unsigned long int) 1234;
  
  printf("align = %ld\n", barrier -> __align);

}
