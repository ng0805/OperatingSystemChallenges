#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "myshell_parser.h"

#define stdIn 0
#define stdOut 1
#define maxArgSize 512

bool errorCheck(struct pipeline *pipe, char *errorMessage);

//void childHandler(int signo);

//void init_sigaction();

int main(int argc, char *argv[]) {
  
  char in[maxArgSize], errorMessage[60];
  
  int status, flowThru[2], redirectIn, redirectOut;
  
  bool doubleRedirectCheck, ampersandPosCheck, dontprintinfail = 0;

  struct pipeline* pipeLine = NULL;

  struct pipeline_command *currentCommand = NULL; //the latest command

  static struct pipeline_command *headCommand = NULL; //head command

  //init_sigaction();
  
  while(true){ //constantly loops and asks for new input
      
    if(argc >= 2 && strcmp(argv[1],"-n")){ //checks for second argument and if "-n" flag was found

      printf("my_shell$"); //prints if >= 2 args, but -n flag not found
    }

    else if(argc < 2){ //prints if only one arg passed

      printf("my_shell$");
    }
    
    if((fgets(in,maxArgSize,stdin)) == NULL && dontprintinfail){ //deal with later

      fprintf(stderr, "ERROR: This shouldnt print\n");
    }

    if (feof(stdin)){ //ctrl+d exit

      printf("\n");

      if(pipeLine != NULL){ //frees pipeline only if its been initalized
      
	pipeline_free(pipeLine);
      }
      
      exit(status);
    }

    pipeLine = pipeline_build(in, &doubleRedirectCheck, &ampersandPosCheck);
    
    if(errorCheck(pipeLine, errorMessage)){// print any of the error messages sent by errorCheck

      fprintf(stderr, "%s", errorMessage);
    }
    
    else if(doubleRedirectCheck){ //check for << or >>

      fprintf(stderr, "ERROR: << or >> not allowed, please use single < or >\n");
    }

    else if(ampersandPosCheck){ //check for & anywhere but end of input

      fprintf(stderr, "ERROR: & may only be inputed at the end of the input\n");
    }

    else{ //only runs the rest of the program if no errors were found in errorCheck

      headCommand = pipeLine -> commands; //keeps track of head command 
      
      currentCommand = pipeLine -> commands; //keeps track of current command
      
      if((pipe(flowThru)) == -1){ //check pipe can open properly

	fprintf(stderr, "ERROR: pipe failed to initialize\n");
      }
      
      while(currentCommand != NULL){

	if(fork() != 0){ //parent

	  if(currentCommand -> next == NULL){ //if on the last command, close pipe write since the output will be stdout or a redirect file

	    close(flowThru[1]);
	    }
	  
	  // if(!pipeLine -> is_background){ //only waits if its not a background process
	  
	    waitpid(-1, &status, 0);
	  // }
	}

	else{ //child
	  
	  if(currentCommand -> redirect_in_path != NULL){ //set redirect in if its present

	    if((redirectIn = open(currentCommand -> redirect_in_path, O_RDONLY | O_CREAT, 0666)) == -1){ //check if redirect in opens properly

	      fprintf(stderr, "ERROR: Open redirect in file failure\n");

	      break;
	    }
	    close(stdIn);

	    dup2(redirectIn, stdIn);

	    close(redirectIn);
	  }

	  if(currentCommand -> redirect_out_path != NULL){ // set redirect out if its present

	    if((redirectOut  = open(currentCommand -> redirect_out_path, O_WRONLY | O_CREAT, 0666)) == -1){ //check if redirect out opens properly

	      fprintf(stderr, "ERROR: Open redirect out file failure\n");

	      break;
	    }

	    close(stdOut);

	    dup2(redirectOut, stdOut);

	    close(redirectOut);
	  }

	  if(currentCommand -> next != NULL){ // set std to pipe write if there is another command in the pipe
	    
	    close(stdOut);

	    dup2(flowThru[1],stdOut);
	  }

	  close(flowThru[1]); //flowThru[1] always closes no matter the situation
	  
	  if(currentCommand != headCommand){ //check that current command is not first command, if not then there is a pipe input
	    
	    close(stdIn);

	    dup2(flowThru[0],stdIn);
	  }

	  close(flowThru[0]); //flowThru[0] alwasy closes no matter the situation
	  
	  execvp(currentCommand->command_args[0],currentCommand->command_args); //execute current command
	}
	
	currentCommand = currentCommand -> next; //move to next command
	
	if(currentCommand == NULL){ //if no more commands, close pipe read since there is no need to anymore

	  close(flowThru[0]); //Note: if statement must be right after moving to next command as while loop will not run if currentCommand = NULL
	}
      }
    }
  }
  return 0;
}

bool errorCheck(struct pipeline *pipe, char *errorMessage){

  bool afterFirstCommand = 0;

  struct pipeline_command *currentCommand = pipe -> commands; //the latest command

  while(currentCommand != NULL){

    if(afterFirstCommand){ //checks for redirect in's for any command after the first one

      if(currentCommand -> redirect_in_path != NULL){

	strcpy(errorMessage, "ERROR: redirect in file only allowed for first command\n");

	return 1;
      }
    }

    else if(currentCommand -> next != NULL){

      if(currentCommand -> redirect_out_path != NULL){

	strcpy(errorMessage, "ERROR: redirect out file only allowed for final command\n");

	return 1;
      }
    }

    afterFirstCommand = 1; //set to 1 only after the first command has been checked

    currentCommand = currentCommand -> next;
  }

  return 0; //returns 0 only if no errors were found
}
/*
void childHandler(int signo)
{
  int status, pid;

    pid = waitpid(0, &status, WNOHANG);

    printf("pid %d process handled\n", pid);
}

void init_sigaction()
{
  struct sigaction action;

  sigemptyset(&action.sa_mask);
  
  action.sa_flags = 0;
  
  action.sa_handler = childHandler;

  sigaction(SIGCHLD, &action, NULL);
}*/
