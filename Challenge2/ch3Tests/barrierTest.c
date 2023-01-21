#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define COUNTER_FACTOR 100000000

pthread_barrier_t barrier;

bool serialCheck = 0, a = 1, b = 1;

int serialCounter = 0;

void *countA(void *arg) {
  unsigned long int c = (unsigned long int)arg;
  int i, check;

  for (i = 0; i < c; i++) {
    
    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n", pthread_self(), i, c);
    }

    if(i == 25000000){

      check = pthread_barrier_wait(&barrier);
    }

    if(check == PTHREAD_BARRIER_SERIAL_THREAD && a){

      a = 0;
      
      serialCheck = !serialCheck;

      serialCounter++;
    }
  }  
  return NULL;
}

void *countB(void *arg) { //delays main counting for to check if barrier properly synchs
  unsigned long int c = (unsigned long int)arg;
  int i, check;

  for(i = 0; i <= c/4;i++){

    if(i == 0){

      printf("1st for loop starting\n");
    }

    else if(i == c/4){

      printf("1st for loop ending\n");
    }
  }

  for (i = 0; i < c; i++) {

    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n", pthread_self(), i, c);
    }

    if(i == 25000000){
      
      check = pthread_barrier_wait(&barrier);
    }

    if(check == PTHREAD_BARRIER_SERIAL_THREAD && b){

      b = 0;
      
      serialCheck = !serialCheck;

      serialCounter++;
    }
  }

  return NULL;
}

int main(int argc, char **argv)
{ 
  pthread_t tid1, tid2;

  pthread_barrier_init(&barrier, NULL, 2);
  
  (void)pthread_create(&tid1, NULL, countA, (void *)(intptr_t) COUNTER_FACTOR);

  (void)pthread_create(&tid2, NULL, countB, (void *)(intptr_t) COUNTER_FACTOR);

  for (int i = 0; i < 200000000; ++i) {
    /* Just wasting time */

    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %d\n", pthread_self(), i, 1000000000);
    }
  }

  printf("Mutex about to destroy\n");

  pthread_barrier_destroy(&barrier);

  printf("serialCheck = %d, counter = %d\n", serialCheck, serialCounter);
  
  printf("Main about to exit\n");

  int a = 10;

  void *p = &a;

  pthread_exit(p);

  return 0;
}
