#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

int counter = 0;

void alarmHandle(int signum){

  counter++;
}

void main(){

  clock_t t;
  t = clock();
  printf("Timer starts\n");
  
  extern int errno;

  int tracker = 0, secs = 0, errnumA,errnumB;

  bool alarmGo = 1;

  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = alarmHandle;

  if(sigaction(SIGALRM, &action, NULL) == -1){

    errnumA = errno;

    fprintf(stderr, "error on the signal handler: %s\n", strerror(errnumA));
  }

  if(ualarm(50000,50000) == -1){

    errnumB = errno;

    fprintf(stderr, "error on the signal handler: %s\n", strerror(errnumB));
  }

  printf("counter = %d\n", counter);
  
  while(1){

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    
    if(time_taken >= 1){
      
      printf("tt = %f, counter = %d\n", time_taken, counter);
      break;
    }
  }
}
