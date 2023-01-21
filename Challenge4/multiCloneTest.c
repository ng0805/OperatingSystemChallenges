#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tls.h"

#define THREAD_CNT 100

#define HAVE_PTHREAD_JOIN 0

#define COUNTER_FACTOR 1000000

#define TEST_ASSERT(x) do { \
  if (!(x)) { \
  fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
	  __FILE__, __LINE__, #x); \
  abort(); \
  } \
  } while(0)

bool create = 1, create2 = 1;

pthread_t tid;

int counter = 0, asdf = 0;

pthread_mutex_t m1;

pthread_barrier_t b1;

void *count(void *arg) {

  char *write = "I",*readA1 = malloc(sizeof(char)*strlen(write)), *readB1 = malloc(sizeof(char)*strlen(write));
  char *write2 = "dont really", *readA2 = malloc(sizeof(char)*strlen(write2)), *readB2 = malloc(sizeof(char)*strlen(write2));
  char *write3 = "want to run", *readA3 = malloc(sizeof(char)*strlen(write3)), *readB3 = malloc(sizeof(char)*strlen(write3));
  char *write4 = "even more tests that keep passing", *readB4 = malloc(sizeof(char)*strlen(write4));
  
  pthread_mutex_lock(&m1);

  printf("thread %d = %ld\n", counter, pthread_self());

  counter++;

  if(create){
    
    create = 0;

    tid = pthread_self();

    printf("Thread %ld creating tls\n", pthread_self());

    TEST_ASSERT((tls_create(61440) != -1));

    printf("Success on TLS creation\n");

    printf("writing\n");

    TEST_ASSERT(tls_write(2000, strlen(write), write) != -1);

    printf("Write success\n");

    printf("writing boundary\n");

    TEST_ASSERT(tls_write(4093, strlen(write3), write3) != -1);

    printf("Boundary write success\n");

    printf("writing on edge\n");

    TEST_ASSERT(tls_write(61438, strlen(write4), write4) == -1);

    printf("Success on edge write failure\n");
  }
  else if(pthread_self() != tid){

    printf("Thread %ld copying thread %ld?\n", pthread_self(), tid);

    create2 = 0;

    TEST_ASSERT(tls_clone(tid) != -1);

    printf("Clone Sucess\n");

    printf("clone write\n");

    TEST_ASSERT(tls_write(100, strlen(write2), write2) != -1);

    printf("COW success\n");

    printf("boundary COW write\n");

    TEST_ASSERT(tls_write(8190, strlen(write4), write4) != -1);

    printf("Boundary COW success\n");

    TEST_ASSERT(tls_write(61438, strlen(write3), write3) == -1);

    printf("Success on edge COW failure\n");
  }
  
  pthread_mutex_unlock(&m1);

  printf("thread %ld barrier waiting\n",pthread_self());

  pthread_barrier_wait(&b1);

  pthread_mutex_lock(&m1);

  if(pthread_self() == tid){

    TEST_ASSERT(tls_read(2000, strlen(write), readA1) != -1);

    TEST_ASSERT(tls_read(100, strlen(write2), readA2) != -1);

    TEST_ASSERT(tls_read(4093, strlen(write3), readA3) != -1);

    printf("readA1, readA2, readA3 successful\n");

    TEST_ASSERT(strcmp(readA1, "I") == 0);

    TEST_ASSERT(strcmp(readA2, "") == 0);

    TEST_ASSERT(strcmp(readA3, "want to run") == 0);

    printf("readA1 = %s, readA2 =%s, readA3 = %s\n", readA1, readA2, readA3);

    TEST_ASSERT(tls_destroy() != -1);

    printf("done destroy\n\n");

    asdf++;
  }

  else{
    
    TEST_ASSERT(tls_read(2000, strlen(write), readB1) != -1);

    TEST_ASSERT(tls_read(100, strlen(write2), readB2) != -1);

    TEST_ASSERT(tls_read(4093, strlen(write3), readB3) != -1);

    TEST_ASSERT(tls_read(8190, strlen(write4), readB4) != -1);

    printf("readB1, readB2, readB3, read B4 success\n");

    TEST_ASSERT(strcmp(readB1, "I") == 0);

    TEST_ASSERT(strcmp(readB2, "dont really") == 0);

    TEST_ASSERT(strcmp(readB3, "want to run") == 0);

    TEST_ASSERT(strcmp(readB4, "even more tests that keep passing") == 0);

    printf("readB1 = %s, readB2 = %s, readB3 = %s, readB4 = %s\n", readB1, readB2, readB3, readB4);

    TEST_ASSERT(tls_destroy() != -1);

    printf("done destroy\n\n");

    asdf++;
  }

  pthread_mutex_unlock(&m1);

  return NULL;
}

int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
  int i;
  pthread_mutex_init(&m1, NULL);
  pthread_barrier_init(&b1, NULL, THREAD_CNT);
  for(i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, count,
		   (void *)(intptr_t)((i + 1) * COUNTER_FACTOR));
  }

  while(asdf < THREAD_CNT){};

  pthread_mutex_destroy(&m1);
  pthread_barrier_destroy(&b1);

  printf("end of main\n");

  return 0;
}
