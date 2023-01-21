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

bool blocked = 0;

struct sigaction action;

void alarmHandle(int signum){

  counter++;

  write(STDOUT_FILENO,"alarm called\n", 13);

  if(counter == 10){

    write(STDOUT_FILENO,"blocking alarm\n", 10);

    blocked = 1;

    sigprocmask(SIG_UNBLOCK, &action.sa_mask, NULL);
  }
}

void main(){

  static int unblock = 0;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_NODEFER;

  action.sa_handler = alarmHandle;

  sigaction(SIGALRM, &action, NULL);

  ualarm(500000,999999);

  while(1){

    if(blocked){

      unblock++;

      if(unblock == 1000000000){

	printf("unblocking alarm\n");

	blocked = 0;

	sigprocmask(SIG_BLOCK, &action.sa_mask, NULL);
      }
    }
  }
}
