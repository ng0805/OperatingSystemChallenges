#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

bool print = 0;

void alarmHandle(int signum){

  print = 1;
}

void *sleeper(void *arg){

  int i;

  printf("Start of the thread function\n");

  while(1){

    //printf("In while loop, print = %d\n", print);

    if(print){

      printf("Thread function hopefully closing\n");

      pthread_exit(&i);
    }
  }
}

void main(){

  int tracker = 0, secs = 0;

  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = alarmHandle;

  sigaction(SIGALRM, &action, NULL);

  ualarm(6000000,3000000);

  pthread_t thread_id;
  printf("Before Thread\n");
  pthread_create(&thread_id, NULL, sleeper, NULL);
  printf("thread created\n");
  /*while(1){

    if(print){

      printf("Signal handler has been called %d times\n",tracker);

      tracker++;

      print = 0;

    }
    printf("Its been %d secs\n", secs);

    secs+=5;

    sleep(5);
    
    printf("done in %d secs\n", secs - 5);
    }*/
}
