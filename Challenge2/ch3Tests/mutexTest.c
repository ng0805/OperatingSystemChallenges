#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define COUNTER_FACTOR 100000000

pthread_mutex_t lock, lock2;

int tracker = 0;

/*static void *launched_function(void *arg)
{
  printf("In function: %ld\n", pthread_self());
  return NULL;
  }*/

void *count(void *arg) {
  unsigned long int c = (unsigned long int)arg;
  int i;
  
  for (i = 0; i < c; i++) {

    if(i == 25000000 && pthread_self() == 2){
      
      pthread_mutex_lock(&lock);

      tracker = 2;
    }

    else if(i == 40000000 && pthread_self() == 1){
      
      pthread_mutex_lock(&lock);

      tracker = 1;
    }

    else if(i == 60000000 && pthread_self() == 2){

      printf("unlocking thread %ld, tracker = %d\n", pthread_self(), tracker);
      
      pthread_mutex_unlock(&lock);
    }

    else if(i == 80000000 && pthread_self() == 1){

      printf("unlocking thread %ld, tracker = %d\n", pthread_self(), tracker);
      
      pthread_mutex_unlock(&lock);
    }
    
    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n", pthread_self(), i, c);
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t tid1, tid2;

  int errorCheck;
  
  errorCheck = pthread_mutex_init(&lock, NULL);

  errorCheck = pthread_mutex_init(&lock2, NULL);
  
  if(errorCheck != 0){

    printf("error in mutex init\n");
  }
  
  (void)pthread_create(&tid1, NULL, count, (void *)(intptr_t) COUNTER_FACTOR);

  (void)pthread_create(&tid2, NULL, count, (void *)(intptr_t) COUNTER_FACTOR);

  for (int i = 0; i < 100000000; ++i) {
    /* Just wasting time */

    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %d\n", pthread_self(), i, 1000000000);
    }

    if(i == 70000000){

      pthread_mutex_lock(&lock2);
    }
  }
  
  printf("Mutex about to destroy\n");

  pthread_mutex_destroy(&lock);

  pthread_mutex_destroy(&lock2);
  
  printf("Main about to exit\n");

  int a = 10;

  void *p = &a;

  pthread_exit(p);
  
  return 0;
}
