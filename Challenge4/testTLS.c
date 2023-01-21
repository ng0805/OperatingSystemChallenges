#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include "tls.h"

/* How many threads (aside from main) to create */
#define THREAD_CNT 2

/* pthread_join is not implemented in homework 2 */
#define HAVE_PTHREAD_JOIN 0

/* Each counter goes up to a multiple of this value. If your test is too fast
 * use a bigger number. Too slow? Use a smaller number. See the comment about
 * sleeping in count() to avoid this size-tuning issue.
 */
#define COUNTER_FACTOR 1000000

#define TEST_ASSERT(x) do { \
  if (!(x)) { \
  fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
	  __FILE__, __LINE__, #x); \
  abort(); \
  } \
  } while(0)

struct page
{
  long unsigned int addr;
  int ref_count;
};

typedef struct thread_local_storage
{
  pthread_t tid;
  unsigned int size;      /* size in bytes                    */
  unsigned int page_num;  /* number of pages                  */
  struct page **pages;    /* array of pointers to pages       */
} TLS;

struct tid_tls_pair
{
  pthread_t tid;
  TLS *tls;
};

bool create = 1, off = 1, on = 0;

pthread_t tid, tid2;

int counter = 0, loop = 0, access = 0;

pthread_mutex_t m1;

pthread_barrier_t b1;

void *count(void *arg) {
  //unsigned long int c = (unsigned long int)arg;
  //int i;
  
  char *write = "I Nick abcdefg",*readA1 = malloc(sizeof(char)*strlen(write)), *readB1 = malloc(sizeof(char)*strlen(write));
  char *write2 = "My angel is the centerfold", *readA2 = malloc(sizeof(char)*strlen(write2)),*readB2 = malloc(sizeof(char)*strlen(write2));
  char *write3 = "Everything is Awesome!", *readA3 = malloc(sizeof(char)*strlen(write3)), *readB3 = malloc(sizeof(char)*strlen(write3));
  char *write4 = "Why arent the test cases working", *readB4 = malloc(sizeof(char)*strlen(write3));  
  
  pthread_mutex_lock(&m1);
  
  printf("thread %d = %ld\n", counter, pthread_self());

  counter++;
  
  if(create){

    create = 0;
    
    tid = pthread_self();
    
    printf("Thread %ld creating tls\n", pthread_self());
    
    //TEST_ASSERT((tls_create(12279) != -1));
    TEST_ASSERT((tls_create(16384) != -1));
    
    access++;
    
    printf("Success on TLS creation\n");
      
    printf("writing\n");
      
    TEST_ASSERT(tls_write(2000, strlen(write), write) != -1);

    printf("Write success\n");

    TEST_ASSERT(tls_read(2000, strlen(write), readA1) != -1);

    printf("A1 read success\n");
    
    printf("writing boundary\n");

    TEST_ASSERT(tls_write(4093, strlen(write3), write3) != -1);

    printf("Boundary write success\n");

    TEST_ASSERT(tls_read(4093, strlen(write3), readA3) != -1);

    printf("A3 read\n");
    
    printf("writing on edge\n");

    //TEST_ASSERT(tls_write(12275, strlen(write4), write4) == -1);
    TEST_ASSERT(tls_write(16382, strlen(write4), write4) == -1);
    
    printf("Success on edge write failure\n");
  }
  
  else if(pthread_self() != tid){

    printf("Thread %ld copying thread %ld?\n", pthread_self(), tid);
      
    tid2 = pthread_self();
      
    TEST_ASSERT(tls_clone(tid) != -1);

    access++;
    
    printf("Clone Sucess\n");
    
    printf("clone write\n");
      
    TEST_ASSERT(tls_write(100, strlen(write2), write2) != -1);
    
    printf("COW success\n");

    TEST_ASSERT(tls_read(100, strlen(write2), readB2) != -1);

    printf("B2 read success\n");
    
    printf("boundary COW write\n");

    TEST_ASSERT(tls_write(8190, strlen(write4), write4) != -1);

    printf("Boundary COW success\n");

    TEST_ASSERT(tls_read(8190, strlen(write4), readB4) != -1);

    printf("B4 read successful\n");
    
    //TEST_ASSERT(tls_write(12275, strlen(write3), write3) == -1);
    TEST_ASSERT(tls_write(16382, strlen(write3), write3) == -1);
    printf("Success on edge COW failure\n");
  }
  
  pthread_mutex_unlock(&m1);

  printf("thread %ld barrier waiting\n\n",pthread_self());
  
  pthread_barrier_wait(&b1);

  pthread_mutex_lock(&m1);
  
  if(pthread_self() == tid){

    while(off){}
    
    /*
    TEST_ASSERT(tls_read(2000, strlen(write), readA1) != -1);

    TEST_ASSERT(tls_read(100, strlen(write2), readA2) != -1);

    TEST_ASSERT(tls_read(4093, strlen(write3), readA3) != -1);

    TEST_ASSERT(tls_read(16382, strlen(write2), readA2) == -1);
    
    printf("readA1, readA2, readA3 successful\n");
    */
    
    TEST_ASSERT(tls_read(100, strlen(write2), readA2) != -1);
    TEST_ASSERT(tls_read(16382, strlen(write2), readA2) == -1);
    
    printf("readA2 x 2 successful\n");

    TEST_ASSERT(strcmp(readA1, "I Nick abcdefg") == 0);

    TEST_ASSERT(strcmp(readA2, "") == 0);

    TEST_ASSERT(strcmp(readA3, "Everything is Awesome!") == 0);
    
    printf("readA1 = %s, readA2 =%s, readA3 = %s\n", readA1, readA2, readA3);

    TEST_ASSERT(tls_destroy() != -1);

    printf("done destroy\n\n");

    loop++;
  }

  else if(pthread_self() == tid2){    
    /*
    TEST_ASSERT(tls_read(2000, strlen(write), readB1) != -1);
    TEST_ASSERT(tls_read(100, strlen(write2), readB2) != -1);
    TEST_ASSERT(tls_read(4093, strlen(write3), readB3) != -1);
    TEST_ASSERT(tls_read(8190, strlen(write4), readB4) != -1);

    printf("readB1, readB2, readB3, read B4 success\n");
    */

    TEST_ASSERT(tls_read(2000, strlen(write), readB1) != -1);
    TEST_ASSERT(tls_read(4093, strlen(write3), readB3) != -1);

    printf("readB1 and readB3 success\n");

    TEST_ASSERT(strcmp(readB1, "I Nick abcdefg") == 0);

    TEST_ASSERT(strcmp(readB2, "My angel is the centerfold") == 0);

    TEST_ASSERT(strcmp(readB3, "Everything is Awesome!") == 0);

    TEST_ASSERT(strcmp(readB4, "Why arent the test cases working") == 0);
    
    printf("readB1 = %s, readB2 = %s, readB3 = %s, readB4 = %s\n", readB1, readB2, readB3, readB4);

    TEST_ASSERT(tls_destroy() != -1);

    printf("done destroy\n\n");
    
    loop++;
  }

  pthread_mutex_unlock(&m1);
  return NULL;
}

void* illegalMemAccess(){

  char *write2 = "My angel is the centerfold", *readA2 = malloc(sizeof(char)*strlen(write2));
  
  TEST_ASSERT(tls_write(100, strlen(write2), write2) == -1);

  printf("write before create failure success\n"); 

  TEST_ASSERT(tls_read(100, strlen(write2), readA2) == -1);

  printf("read before create failure sucess\n");
  
  struct tid_tls_pair ttp = checkSegFault();

  printf("\n\n\nTID = %ld\n\n\n", ttp.tls -> tid);

  printf("pages = %ld\n", (unsigned long int) ttp.tls -> pages[0]);
  
  unsigned long int addr = ttp.tls -> pages[0] -> addr;

  char *write = "This should fail";

  printf("\nExit Thread = %ld\n\n", pthread_self());

  loop++;

  off = 0;
  
  memcpy((void*) addr, write, strlen(write));
  
  //raise(SIGSEGV);
  
  //raise(SIGBUS);
  
  printf("\nI should not be here\n");

  return NULL;
}

/*
 * Expected behavior: THREAD_CNT number of threads print increasing numbers
 * in a round-robin fashion. The first thread finishes soonest, and the last
 * thread finishes latest. All threads are expected to reach their maximum
 * count.
 *
 * This isn't a great test for `make check` automation, since it doesn't
 * actually fail when things go wrong. Consider adding lock() and unlock()
 * functions to let your threads "pause" the scheduler occasionally to
 * compare the current state of each thread's counters (you may also need to
 * have an array of per-thread counters). See man pages for sigprocmask,
 * sigemptyset, and sigaddset.
 */
int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
  pthread_t exitThread;
  int i;
  pthread_mutex_init(&m1, NULL);
  pthread_barrier_init(&b1, NULL, THREAD_CNT);
  for(i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, count,
		   (void *)(intptr_t)((i + 1) * COUNTER_FACTOR));
  }

  pthread_create(&exitThread, NULL, illegalMemAccess, NULL);
  
  while(loop < THREAD_CNT+1){
  };

  pthread_mutex_destroy(&m1);
  pthread_barrier_destroy(&b1);

  printf("end of main\n");
  
  return 0;
}
