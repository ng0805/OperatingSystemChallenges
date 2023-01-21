#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define COUNTER_FACTOR 100000000


/*static void *launched_function(void *arg)
{
  printf("In function: %ld\n", pthread_self());
  return NULL;
  }*/

void *count(void *arg) {
  unsigned long int c = (unsigned long int)arg;
  int i;

  printf("in count\n");

  for (i = 0; i < c; i++) {
    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n", pthread_self(), i, c);
    }

    /*if((i%250000000) == 0 && i != 0){
      ualarm(1,0);
      }*/

    if(i == 1){

      printf("start of count for loop\n");
    }

    else if(i == 99999999){

      printf("end of count for loop\n");
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t tid1, tid2;
  /*
   * Normally we'd care to test the return value of pthread_create. But
   * this is meant to be one of the most simple test cases, so let's not
   * over-define it.
   */
  (void)pthread_create(&tid1, NULL, count, (void *)(intptr_t) COUNTER_FACTOR);

  (void)pthread_create(&tid2, NULL, count, (void *)(intptr_t) COUNTER_FACTOR);

  for (int i = 0; i < 100000000; ++i) {
    /* Just wasting time */

    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %d\n", pthread_self(), i, 1000000000);
    }

    if(i == 1){

      printf("start of for loop\n");
    }

    /*else if((i%250000000) == 0 && i != 0){
      ualarm(1,0);
      }*/

    else if(i == 99999999){

      printf("end of for loop\n");
    }
  }

  /* We should not reach here if the test succeeds. */
  printf("Main about to exit\n");

  return 0;
}
