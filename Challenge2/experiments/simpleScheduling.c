#include "ec440threads.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

/* You can support more threads. At least support this many. */
#define MAX_THREADS 128

/* Your stack should be this many bytes in size */
#define THREAD_STACK_SIZE 32767

/* Offset for jump buf stack pointer*/
#define STACK_OFFSET (THREAD_STACK_SIZE/8) - 1

/* Number of microseconds between scheduling events */
#define SCHEDULER_INTERVAL_USECS (50 * 1000)

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

/* thread_status identifies the current state of a thread. You can add, rename,
 * or delete these values. This is only a suggestion. */
enum thread_status
  {
   TS_EXITED,
   TS_RUNNING,
         TS_READY
  };


/* The thread control block stores information about a thread. You will
 * need one of this per thread.
 */
struct thread_control_block {
  pthread_t threadID;
  jmp_buf buf;
  int bufReturnVal; //uses threadID for longjmp return val
  unsigned long *endStackPtr;
  enum thread_status status;
};

//Global Vars
pthread_t currentThreadID; //current thread id set by scheduler and returnd by pthread_self
struct thread_control_block tcbList[MAX_THREADS]; //array of threads
int numOfThreads = 0; //used to track current number of threads and current position in tcbList
int counter = 0; //Get rid
clock_t t, h;

/*static void saveMainBuf(){ //Utility function for saving main buffer
  static bool firstCall = 1;
  if(firstCall){
    
    tcbList[0].threadID = 0; //Sets up TCB for Main thread
    tcbList[0].bufReturnVal = 131914; //bufReturnVal cant be 0 since thats the default return, 131914 = "MAIN"
    tcbList[0].endStackPtr = NULL; //stack pointer not needed for main thread
   
    tcbList[0].status = TS_READY;
  }
  
  setjmp(tcbList[0].buf);
  }*/

// to supress compiler error
static void schedule(int signal) __attribute__((unused));

static void schedule(int signal)
{
  static int topThread = 0; //starts at 1 and cycles through the list of threads each time schedule is called

  //static bool firstCall = 1; //checks if after first schedule call;

  if(tcbList[topThread].status != TS_EXITED){ //set status back to ready if thread is not exited if after first sched call

    tcbList[topThread].status = TS_READY;
  }

  /*if(topThread == 1){
    printf("jumping to 0\n");
    ualarm(0,0);
    
    longjmp(tcbList[0].buf, tcbList[0].bufReturnVal); //jump to next thread 
   
    }*/

  /*if(firstCall){ //If first call to scheduler, jump to thread 1

    firstCall = 0;

    currentThreadID = (pthread_t) topThread;

    tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

    printf("jumping to thread %d\n", topThread);

    longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
    }*/

  if(tcbList[topThread].status == TS_EXITED){

    do{//increment to next thread and keep looping until non exited thread is found

      printf("in do-while\n");

      topThread++;

      //printf("tt = %d\n", topThread);

    }while(tcbList[topThread].status == TS_EXITED);

    currentThreadID = (pthread_t) topThread;

    tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

    printf("jumping to thread %d\n", topThread);

    longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
  }

  else{

    if(setjmp(tcbList[topThread].buf) == 0){ //save the state of the current thread, will not go through this after longjmp

      do{ //increment to next thread and keep looping until non exited thread is found

	printf("in do-while\n");

	topThread++;

	if(topThread == numOfThreads+1){ //set back to index 1 if past max number of threads

	  topThread = 0;
	}

	printf("tt = %d\n", topThread);

      }while(tcbList[topThread].status == TS_EXITED);

      if(topThread == 0){

	printf("hi\n");
      }

      currentThreadID = (pthread_t) topThread;

      tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

      printf("jumping to thread %d\n", topThread);

      longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
    }
  }
}

static void saveMainBuf(){ //Utility function for saving main buffer

  tcbList[0].threadID = 0; //Sets up TCB for Main thread

  tcbList[0].bufReturnVal = 131914; //bufReturnVal cant be 0 since thats the default return, 131914 = "MAIN"

  tcbList[0].endStackPtr = NULL; //stack pointer not needed for main thread

  tcbList[0].status = TS_READY;
}

static void scheduler_init()
{
  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = schedule;

  sigaction(SIGALRM, &action, NULL);

  ualarm(SCHEDULER_INTERVAL_USECS,SCHEDULER_INTERVAL_USECS);

  //ualarm(999999, 999999);

  saveMainBuf();
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){

  static bool is_first_call = true;

  if(numOfThreads++ >= MAX_THREADS){ //if at max number of threads, return error

    fprintf(stderr, "ERROR: Max amount of threads reached\n");

    return -1;
  }

  *thread = (pthread_t) numOfThreads;

  printf("pthread_create %d\n", numOfThreads);

  unsigned long* eSP = (unsigned long*) malloc(THREAD_STACK_SIZE);

  if(eSP == NULL){ //create stack and return -1 if not enough memory

    fprintf(stderr, "ERROR: Malloc error, not enough memory\n");

    return -1;
  }

  eSP[STACK_OFFSET] = (unsigned long) pthread_exit;

  void *topStackPtr = eSP + STACK_OFFSET; //set pointer at the top of the stack

  tcbList[numOfThreads].threadID = *thread;

  tcbList[numOfThreads].bufReturnVal = numOfThreads;

  tcbList[numOfThreads].endStackPtr = eSP;

  setjmp(tcbList[numOfThreads].buf);

  tcbList[numOfThreads].buf->__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)start_thunk);
  tcbList[numOfThreads].buf->__jmpbuf[JB_RSP] = ptr_mangle((unsigned long int)topStackPtr);
  tcbList[numOfThreads].buf->__jmpbuf[JB_R12] = (unsigned long int)start_routine;
  tcbList[numOfThreads].buf->__jmpbuf[JB_R13] = (unsigned long int)arg;

  tcbList[numOfThreads].status = TS_READY;

  printf("Thread %d og buf stack = %ld, malloced stack = %ld\n", numOfThreads, ptr_demangle(tcbList[numOfThreads].buf->__jmpbuf[JB_RSP]), (unsigned long int)topStackPtr);

  if (is_first_call){

    is_first_call = false;

    scheduler_init();
  }

  printf("about to return from  pthread_create\n");

  return 0;
}

void pthread_exit(void *value_ptr)
{
  static int exitedThreads = 0; //counts number of threads exited

  int currentThread = (int) currentThreadID; //cast to int for proper array indexing

  printf("exiting thread %d\n", currentThread);

  tcbList[currentThread].status = TS_EXITED;

  if(currentThread != 0){ //dont free memory and increment exited threads if pthread_exit is called from main

    free(tcbList[currentThread].endStackPtr);

    exitedThreads++;
  }

  printf("et = %d, not = %d\n", exitedThreads, numOfThreads);

  if(exitedThreads != numOfThreads){ //If all threads have been exited, exit program

    schedule(0);
  }

  else if(tcbList[0].status != TS_EXITED){ //jump back to main if all other threads but it are finished

    printf("jumping to main\n");

    longjmp(tcbList[0].buf, tcbList[0].bufReturnVal);
  }

  exit(0);
}

pthread_t pthread_self(void)
{
  return currentThreadID;
}
