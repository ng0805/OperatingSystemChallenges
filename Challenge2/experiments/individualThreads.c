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
int numOfThreads = 1; //used to track current number of threads and current position in tcbList

// to supress compiler error
static void schedule(int signal) __attribute__((unused));

static void schedule(int signal)
{
  /*static int topThread = 1; //starts at 1 and cycles through the list of threads each time schedule is called

  if(tcbList[topThread].status != TS_EXITED){ //set status back to ready if thread is not exited

    tcbList[topThread].status = TS_READY;
  }
  
  if(setjmp(tcbList[topThread].buf) == 0){ //save the state of the current thread, will not go through this after longjmp

    do{ //increment to next thread and keep looping until non exited thread is found

      topThread++;
      
      else if(topThread == numOfThreads+1){ //set back to index 1 if past max number of threads

           topThread = 0;
      }
      
    }while(tcbList[topThread].status != TS_EXITED);

    currentThreadID = topThread;
    
    tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

    printf("jumping to thread %d", topThread);
    
    longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
    }*/
}

static void scheduler_init()
{

  /*struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = schedule;

  sigaction(SIGALRM, &action, NULL);

  ualarm(999999,999999);*/

  static bool jumpTo1 = 1;

  static bool first = 1;

  if(first){
  
  tcbList[0].threadID = 0; //Sets up TCB for Main thread

  tcbList[0].bufReturnVal = 131914; //bufReturnVal cant be 0 since thats the default return, 131914 = "MAIN"

  tcbList[0].endStackPtr = NULL; //stack pointer not needed for main thread

  tcbList[0].status = TS_READY;
  }
  
  if(setjmp(tcbList[0].buf) == 0){

      if(jumpTo1){
    
	currentThreadID = 1;
    
	printf("in sched init, jumping to %ld\n", currentThreadID);

	jumpTo1 = 0;
    
	longjmp(tcbList[1].buf, tcbList[1].bufReturnVal); //jump to next thread
      }

      else{

	currentThreadID = 2;

	printf("in sched init, jumping to %ld\n", currentThreadID);

	longjmp(tcbList[2].buf, tcbList[2].bufReturnVal); //jump to next thread  
      }
  }
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){

  static bool is_first_call = true;

  int size = (THREAD_STACK_SIZE/8) - 1; //index size

  unsigned long *eSP = (unsigned long*) malloc(sizeof(THREAD_STACK_SIZE)); //create stack

  void *topStackPtr = eSP + size; //set pointer at the top of the stack

  eSP[size] = (unsigned long) pthread_exit; //put pthread_exit at top of the stack

  *thread = numOfThreads;

  tcbList[numOfThreads].threadID = *thread;

  tcbList[numOfThreads].bufReturnVal = numOfThreads;

  tcbList[numOfThreads].endStackPtr = eSP;

  setjmp(tcbList[numOfThreads].buf);

  tcbList[numOfThreads].buf->__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)start_thunk);
  tcbList[numOfThreads].buf->__jmpbuf[JB_RSP] = ptr_mangle((unsigned long int)topStackPtr);
  tcbList[numOfThreads].buf->__jmpbuf[JB_R12] = (unsigned long int)start_routine;
  tcbList[numOfThreads].buf->__jmpbuf[JB_R13] = (unsigned long int)arg;

  tcbList[numOfThreads].status = TS_READY;

  numOfThreads++; //increment number of threads

  //if (is_first_call){

  //is_first_call = false;

    scheduler_init();
    //}

  return 0;

  //return -1; Check for erro
}

void pthread_exit(void *value_ptr)
{

  static int exitedThreads = 0;

  static bool mainJump = 1;
  
  printf("exiting thread %ld\n", currentThreadID);

  tcbList[currentThreadID].status = TS_EXITED;

  free(tcbList[currentThreadID].endStackPtr);

  exitedThreads++;

  if(exitedThreads != 2){

      printf("jumping to 0\n");
      
      longjmp(tcbList[0].buf, tcbList[0].bufReturnVal); //jump to next thread
  }

  exit(0);
  /* TODO: Exit the current thread instead of exiting the entire process.                                                                                         
   * Hints:                                                                                                                                                       
   * - Release all resources for the current thread. CAREFUL though.                                                                                              
   *   If you free() the currently-in-use stack then do something like                                                                                            
   *   call a function or add/remove variables from the stack, bad things                                                                                         
   *   can happen.                                                                                                                                                
   * - Update the thread's status to indicate that it has exited                                                                                                  
   * - The process shall exit with an exit status of 0 after the last thread has been terminated
   */
}

pthread_t pthread_self(void)
{
  return currentThreadID;
}
