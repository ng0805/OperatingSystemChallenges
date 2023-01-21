#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

void alarmHandle(int signum){

  write(STDOUT_FILENO,"alarm called\n", strlen("alarm called\n"));
  
}

void main(){

  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_NODEFER;
  action.sa_handler = alarmHandle;
  sigaction(SIGALRM, &action, NULL);
  
  raise(SIGALRM);
  
  if(sigprocmask(SIG_BLOCK, &action.sa_mask, NULL) == -1){

    fprintf(stderr, "error on the block: %s\n", strerror(errno));
  }

  else{

    printf("not block\n");
  }

  raise(SIGALRM);
  
  if(sigprocmask(SIG_UNBLOCK, &action.sa_mask, NULL) == -1){

    fprintf(stderr, "error on the unblock: %s\n", strerror(errno));
  }

  else{

    printf("not unblock\n");
  }

  raise(SIGALRM);
}
