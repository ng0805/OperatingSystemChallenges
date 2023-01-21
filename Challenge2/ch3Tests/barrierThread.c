#include "ec440threads.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

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
   TS_READY,
      TS_BLOCKED
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

struct threadList{

  pthread_t threadID;

  struct threadList* next;
};

struct counts{ //keeps track of both max count set by barrier init and current count of threads that have hit barrier

  int maxCount;

  int currentCount;

  bool printSerialThread;
};

//Global Vars
pthread_t currentThreadID; //current thread id set by scheduler and returnd by pthread_self

int numOfThreads = 0; //used to track current number of threads and current position in tcbList 

struct thread_control_block tcbList[MAX_THREADS]; //array of threads

struct threadList* mutexThreadLists[MAX_THREADS]; //indexed into by __data.__count and contains linked list of threads waiting to obtain lock

struct threadList* barrierThreadLists[MAX_THREADS]; //indexed into by __align and contains linked list of threads waiting on barrier

struct counts barrierCounts[MAX_THREADS]; //indexed into by __align and keeps track of count for corresponding barrier

// to supress compiler error
static void schedule(int signal) __attribute__((unused));

static void schedule(int signal)
{
  static int topThread = 0; //starts at 1 and cycles through the list of threads each time schedule is called

  if(tcbList[topThread].status != TS_EXITED && tcbList[topThread].status != TS_BLOCKED){ //set status back to ready if thread is not exited if after first sched call

    tcbList[topThread].status = TS_READY;
  }

  if(tcbList[topThread].status == TS_EXITED){

    do{//increment to next thread and keep looping until non exited thread is found

      topThread++;

      if(topThread == numOfThreads+1){ //set back to index 1 if past max number of threads

	topThread = 0;
      }

    }while(tcbList[topThread].status == TS_EXITED || tcbList[topThread].status == TS_BLOCKED);

    currentThreadID = (pthread_t) topThread;

    tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

    longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
  }

  else{

    if(setjmp(tcbList[topThread].buf) == 0){ //save the state of the current thread, will not go through this after longjmp

      do{ //increment to next thread and keep looping until non exited thread is found

	topThread++;

	if(topThread == numOfThreads+1){ //set back to index 1 if past max number of threads

	  topThread = 0;
	}

      }while(tcbList[topThread].status == TS_EXITED || tcbList[topThread].status == TS_BLOCKED);

      currentThreadID = (pthread_t) topThread;

      tcbList[topThread].status = TS_RUNNING; //set status of next thread to running

      longjmp(tcbList[topThread].buf, tcbList[topThread].bufReturnVal); //jump to next thread
    }
  }
}

static void scheduler_init()
{
  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = schedule;

  sigaction(SIGALRM, &action, NULL);

  if(ualarm(SCHEDULER_INTERVAL_USECS,SCHEDULER_INTERVAL_USECS) == -1){

    fprintf(stderr, "error on alarm initialization: %s\n", strerror(errno));

    exit(-1);
  }

  tcbList[0].threadID = 0; //Sets up TCB for Main thread

  tcbList[0].bufReturnVal = 131914; //bufReturnVal cant be 0 since thats the default return, 131914 = "MAIN"

  tcbList[0].endStackPtr = NULL; //stack pointer not needed for main thread

  tcbList[0].status = TS_READY;
}


static void lock(){

  if(ualarm(0,0) == -1){ //NEED TO CHANGE WITH sigprocmask ONCE FIGURED OUT

    fprintf(stderr, "error on alarm blocking: %s\n", strerror(errno));

    exit(-1);
  }
}

static void unlock(){

  if(ualarm(SCHEDULER_INTERVAL_USECS,SCHEDULER_INTERVAL_USECS) == -1){ //NEED TO CHANGE WITH sigprocmask ONCE FIGURED OUT

    fprintf(stderr, "error on alarm unblocking: %s\n", strerror(errno));

    exit(-1);
  }
}

int pthread_mutex_init( pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr){

  static int id = 0; // keeps track of id for initializing mutex

  mutex ->__data.__count= id; //index into mutexThreadList array, the index contains the linked list holding all the threads waiting for the corresponding lock

  mutex -> __data.__lock = 1; //acts as bool telling whether the lock has been claimed or not

  mutexThreadLists[mutex -> __data.__count] = NULL;

  id++;

  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex){

  struct threadList *head = mutexThreadLists[mutex -> __data.__count];

  struct threadList *temp;

  while(head != NULL){//free all the nodes in the linked list

    temp = head;

    head = head -> next;

    free(temp);
  }
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex){

  printf("attempting to lock mutex %d\n", mutex -> __data.__count);

  lock(); //stop alarm

  if(mutex -> __data.__lock){ //claim the lock

    mutex -> __data.__lock = 0;

    mutex -> __data.__owner = (int) currentThreadID;

    printf("mutex %d is locked and belongs to thread %d\n", mutex -> __data.__count, mutex -> __data.__owner);

    unlock();
  }

  else{

    printf("Blocking thread %ld and adding to queue for mutex %d\n", currentThreadID, mutex -> __data.__count);

    tcbList[currentThreadID].status = TS_BLOCKED;

    printf("Thread %ld is %d\n", currentThreadID, tcbList[currentThreadID].status);

    struct threadList *thread = (struct threadList*) malloc(sizeof(struct threadList)); //create new threadList node to add to linked list for mutex

    struct threadList *current = mutexThreadLists[mutex -> __data.__count];

    thread -> threadID = currentThreadID;

    thread -> next = NULL;

    if(current == NULL){ //if linked list is empty, insert node at head

      mutexThreadLists[mutex -> __data.__count] = thread;
    }

    else{ //if linked list has items, find last thread and put next thread after that

      while(current -> next != NULL){

	current = current -> next;
      }

      current -> next = thread;
    }

    unlock();

    schedule(14); //schedule so thread doesn't keep running
  }

  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){ //what to do after unblocking thread

  printf("unlocking lock %d\n", mutex -> __data.__count);

  lock();

  struct threadList *current = mutexThreadLists[mutex -> __data.__count];

  int threadID;

  if(current == NULL){ //if there are no more threads waiting for the lock

    mutex -> __data.__lock = 0;

    mutex -> __data.__owner = 149311; //149311 means no owner

    unlock();
  }

  else{ //move to next thread

    threadID = current -> threadID;// get id of next thread waiting for the lock

    tcbList[threadID].status = TS_READY; //unblock next thread

    current = current -> next;

    mutexThreadLists[mutex -> __data.__count] = current; //pop off top thread and set next thread to the top

    printf("Thread %d unblocked, scheduling now\n", threadID);

    unlock();
  }

  return 0;
}

int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count){

  printf("barrier init\n");
  
  static int id = 0;

  if(count == 0){

    return EINVAL;
  }
  
  barrier -> __align = id; //set id of barrier

  barrierCounts[barrier -> __align].maxCount = count; //set counter for barrier

  barrierCounts[barrier -> __align].currentCount = 0; //set number of threads hit to 0;

  barrierCounts[barrier -> __align].printSerialThread = 1;
  
  barrierThreadLists[barrier -> __align] = NULL; //set empty linked list at first

  id++; //increment id for next barrier
  
  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier){

  printf("barrier %ld destroy\n", barrier -> __align);

  struct threadList *head = barrierThreadLists[barrier -> __align]; //get head of thread LL

  struct threadList *temp;

  while(head != NULL){//free all the nodes in the linked list

    temp = head;

    head = head -> next;

    free(temp);
  }
  
  return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier){

  printf("barrier %ld wait thread %ld\n", barrier -> __align, currentThreadID);

  lock();
  
  struct threadList *current = barrierThreadLists[barrier -> __align]; //get head of thread LL 

  barrierCounts[barrier -> __align].currentCount++; //increment currentCount by 1 
  
  if(barrierCounts[barrier -> __align].printSerialThread == 0){ //set printSerialThread back to 1, should only happen if a barrier is beign reused as its only                                                                     set to 0 then exiting
    barrierCounts[barrier -> __align].printSerialThread = 1;
  }
  
  if(barrierCounts[barrier -> __align].maxCount != barrierCounts[barrier -> __align].currentCount){ //if maxCount of threads has not been met

    tcbList[currentThreadID].status = TS_BLOCKED;

    struct threadList *thread = (struct threadList*) malloc(sizeof(struct threadList)); //create new threadList node to add to linked list for mutex

    thread -> threadID = currentThreadID;

    thread -> next = NULL;

    if(current == NULL){ //if linked list is empty, insert node at head

      barrierThreadLists[barrier -> __align] = thread;
    }

    else{ //if linked list has items, find last thread and put next thread after that

      while(current -> next != NULL){

	current = current -> next;
      }

      current -> next = thread;
    }

    unlock();

    schedule(14); //schedule after blocked thread
  }

  else{ //if count # of threads has been reached

    barrierCounts[barrier -> __align].currentCount = 0; //reset current count so barrier can be reused
    
    while(current != NULL){ //set all blocked threads back to ready

      tcbList[current -> threadID].status = TS_READY;

      current = current -> next;
    }

    unlock();
  }

  if(barrierCounts[barrier -> __align].printSerialThread){ //print PTHREAD_BARRIER_SERIAL_THREAD once per barrier exit

    barrierCounts[barrier -> __align].printSerialThread = 0;
    
    return PTHREAD_BARRIER_SERIAL_THREAD;
  }

  else{
  
    return 0;
  }
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){

  static bool is_first_call = true;

  if(numOfThreads++ >= MAX_THREADS){ //if at max number of threads, return error

    fprintf(stderr, "ERROR: Max amount of threads reached\n");

    return -1;
  }

  *thread = (pthread_t) numOfThreads;

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

  if (is_first_call){

    is_first_call = false;

    scheduler_init();
  }

  return 0;
}

void pthread_exit(void *value_ptr)
{
  static int exitedThreads = 0; //counts number of threads exited

  int currentThread = (int) currentThreadID; //cast to int for proper array indexing

  tcbList[currentThread].status = TS_EXITED;

  if(currentThread != 0){ //dont free memory and increment exited threads if pthread_exit is called from main

    free(tcbList[currentThread].endStackPtr);

    exitedThreads++;
  }

  if(exitedThreads != numOfThreads){ //If all threads have been exited, exit program

    schedule(0);
  }

  else if(tcbList[0].status != TS_EXITED){ //jump back to main if all other threads but it are finished

    currentThreadID = 0;

    tcbList[0].status = TS_RUNNING;

    longjmp(tcbList[0].buf, tcbList[0].bufReturnVal);
  }

  exit(0);
}

pthread_t pthread_self(void)
{
  return currentThreadID;
}
