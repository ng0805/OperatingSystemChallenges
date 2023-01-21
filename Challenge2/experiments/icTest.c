#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int counter = 1;

static void *launched_function(void *arg)
{
  printf("In function: %d\n", counter);
}

int main(int argc, char **argv)
{
  pthread_t tid1, tid2;
  /*
   * Normally we'd care to test the return value of pthread_create. But
   * this is meant to be one of the most simple test cases, so let's not
   * over-define it.
   */
  (void)pthread_create(&tid1, NULL, launched_function, NULL);

  (void)pthread_create(&tid2, NULL, launched_function, NULL);
  
  for (int i = 0; i < 1000000000; ++i) {
    /* Just wasting time */
  }

  /* We should not reach here if the test succeeds. */
  printf("should not be here\n");
  
  exit(-1);
}
