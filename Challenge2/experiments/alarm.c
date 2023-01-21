#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

bool print = 0;

void alarmHandle(int signum){

  printf("handle");
  
  print = 1;
}

void main(){

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
  
  while(1){
    
    if(print){

      printf("Signal handler has been called %d times\n",tracker);

      print = 0;

      tracker++;
    }
    printf("Its been %d secs\n", secs);

    secs+=5;

    sleep(5);

    if(alarmGo){

      printf("activating alarm");
      
      if(ualarm(1,999999) == -1){

	errnumB = errno;

	fprintf(stderr, "error on alarm: %s\n", strerror(errnumB));
      }
    }

    alarmGo = 0;
    
    printf("done in %d secs\n", secs);
  }
}
