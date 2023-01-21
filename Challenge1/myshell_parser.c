#include "myshell_parser.h"
#include "stddef.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*Called when a '|' is found in the command input and ddds a new node to the command list
  the latest command is given and the functions return the newly created command*/

struct pipeline_command *newCommand(struct pipeline_command *currentCommand);

/*The function that adds the current character from command_line to either the argument, 
  redirect in, or redirect out depending on boolean inputs, letter is
  the character that will be added, and position is the position in the array where the
  letter will be added*/

void characterInsert(char *argStore, char *redirectIn, char *redirectOut, bool inCheck, bool outCheck, char letter, int *argStorePos, int *redirectInPos, int *redirectOutPos);


struct pipeline *pipeline_build(const char *command_line, bool *doubleRedirectCheck, bool *ampersandPosCheck)
{
  int argStorePos = 0, commandArgPos = 0, redirectInPos = 0, redirectOutPos = 0; /*storePos keeps track of current position in the specified array
										   xPos keeps position of current string in x array*/
  int riCheck = 0, roCheck = 0, inputPos = 0, inSize = (int) strlen(command_line); //error checking integers

  bool inCheck = 0, outCheck = 0, mallocCheck = 1; /*in/outCheck check for an in/out redirectPath, nextCommand goes true when '|' is found
						       mallocCheck is a bool that allows the redirect in/out to be malloced once per command*/
  struct pipeline * pipe = NULL;

  struct pipeline_command *headCommand = NULL; //The top command

  struct pipeline_command *currentCommand = NULL; //the latest command

  pipe = (struct pipeline*) malloc(sizeof(struct pipeline));

  headCommand = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));

  currentCommand = headCommand; //sets to head in the beginning

  pipe -> commands = headCommand; //set commands in pipe to headCommand

  currentCommand -> command_args[0] = (char*) malloc(sizeof(char[30])); //initialize first string in the command array;

  *currentCommand -> command_args[0] = '&';

  *doubleRedirectCheck = 0; //set error checks to 0 by default and will be changed to 1 if error is encountered

  *ampersandPosCheck = 0;

  while(*command_line != '\n'){

    if(mallocCheck){

      currentCommand -> redirect_in_path = (char*) malloc(sizeof(char[30])); /*since there will not always be redirects, but the code consitently checks for redirects
                                                                               the paths need to be allocated once per command in order to prevent seg faults*/
      currentCommand -> redirect_in_path[0] = '&'; /*set to '&' since that char can never be added to the redirects normally
						     used to check if redirect has been filled or not*/
      currentCommand -> redirect_out_path = (char*) malloc(sizeof(char[30]));

      currentCommand -> redirect_out_path[0] = '&';

      mallocCheck = false;
    }

    if(*command_line == '&'){

      pipe -> is_background = true;

      if(inputPos != inSize-2){ //if ampersand is found anywhere but end of input then set ampersandPosCheck and stop parsing

	*ampersandPosCheck = 1;

	break;
      }
    }

    else if(*command_line == '<'){

      inCheck = true; //only one direct can be filled at a time

      outCheck = false;

      riCheck++; //riCheck is incremented every time '<' is found and will set an error if more than 1 is found

      if(riCheck > 1){ //set doubleRedirectCheck to true if more than one < is found and stop parsing

	*doubleRedirectCheck = 1;

	break;
      }
    }

    else if(*command_line == '>'){

      outCheck = true;

      inCheck = false;

      roCheck++; //roCheck is incremented every time '>' is found and will set an error if more than 1 is found

      if(roCheck > 1){ //set doubleRedirectCheck to true if more than one > is found and stop parsing

	*doubleRedirectCheck = 1;

	break;
      }
    }

    else if(*command_line == '|'){

      if(*currentCommand -> command_args[commandArgPos] == '&'){ //if a space was encountered before EoF, commandArgPos was already incremented

	currentCommand -> command_args[commandArgPos] = NULL; //set next command = NULL
      }

      else{ //if a space wasn't encountered before EOF

	currentCommand -> command_args[commandArgPos+1] = NULL;
      }

      if(currentCommand -> redirect_in_path[0] == '&'){ //if redirect has not been filled, set to NULL

	currentCommand -> redirect_in_path = NULL;
      }

      if(currentCommand ->redirect_out_path[0] == '&'){ //redirect in/out are both possible in a single command so the if's need to be independent

	currentCommand -> redirect_out_path = NULL;
      }

      currentCommand = newCommand(currentCommand);

      mallocCheck = true; //new command will need its redirects allocated for proper checks;

      argStorePos = 0; //set all positions in arrays back to 0

      redirectInPos = 0;

      redirectOutPos = 0;

      commandArgPos = 0;

      inCheck = false;

      outCheck = false;
    }

    else if(*command_line == ' '){ //increments argument to next position in arg array if a space is encountered

      if(*(currentCommand -> command_args[commandArgPos]+0) != '&' && !inCheck && !outCheck){ /*if there is a redrect this does not matter, but there can't be a character add, so the else if still needs to run
                                                                                                must check if the current arguemnt has been filled, before moving onto the next one, used to check for multiple spaces between args*/
	commandArgPos++;

	currentCommand -> command_args[commandArgPos] = (char*) malloc(sizeof(char[30])); //move to next argument and allocate space

	*(currentCommand -> command_args[commandArgPos]+0) = '&';

	argStorePos = 0; //go back to 0 index for next command array
      }
    }

    else{ //adds a non whitespace character to current arguement

      characterInsert(currentCommand -> command_args[commandArgPos], currentCommand ->redirect_in_path, currentCommand -> redirect_out_path, inCheck, outCheck, *command_line, &argStorePos, &redirectInPos, &redirectOutPos);
    }

    inputPos++; //track current position in the full input;

    command_line++;
  }

  if(currentCommand -> redirect_in_path[0] == '&'){ //if redirect has not been filled, set to NULL

    currentCommand -> redirect_in_path = NULL;
  }

  if(currentCommand ->redirect_out_path[0] == '&'){ //redirect in/out are both possible in a single command so the if's need to be independent

    currentCommand -> redirect_out_path = NULL;
  }

  if(*currentCommand -> command_args[commandArgPos] == '&'){ //if a space was encountered before EoF, commandArgPos was already incremented

    currentCommand -> command_args[commandArgPos] = NULL;
  }

  else{ //if a space wasn't encountered before EOF

    currentCommand -> command_args[commandArgPos+1] = NULL;
  }
  
  currentCommand -> next = NULL;

  return pipe;
}

void pipeline_free(struct pipeline *pipeline)
{
  int commandArrayIter; //keeps track of current position in command_args

  char *command; //points to current command in command_args to free

  struct pipeline_command *tempCommand;
  
  while(pipeline -> commands != NULL){

    tempCommand = pipeline -> commands;

    commandArrayIter = 0;

    command = tempCommand -> command_args[commandArrayIter];

    while(command != NULL){ //iterate through command_args to free until NULL is encountered

      free(command);

      commandArrayIter++; //move to next position

      command = tempCommand -> command_args[commandArrayIter];
    }

    free(tempCommand -> redirect_in_path);

    free(tempCommand -> redirect_out_path);

    pipeline -> commands = pipeline -> commands -> next;

    free(tempCommand);
  }

  free(pipeline);
}

struct pipeline_command *newCommand(struct pipeline_command *currentCommand){

  struct pipeline_command *tempCommand = NULL;

  tempCommand = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));

  currentCommand -> next = tempCommand;

  tempCommand -> command_args[0] = (char*) malloc(sizeof(char[30]));

  *tempCommand -> command_args[0] = '&';

  return tempCommand;
}

void characterInsert(char *argStore, char *redirectIn, char *redirectOut, bool inCheck, bool outCheck, char letter, int *argStorePos, int *redirectInPos, int *redirectOutPos){

  if(inCheck){

    *(redirectIn + *redirectInPos) = letter;

    *redirectInPos+=1; //increment to next position
  }

  else if(outCheck){

    *(redirectOut + *redirectOutPos) = letter;

    *redirectOutPos+=1;
  }

  else{

    *(argStore + *argStorePos) = letter;

    *argStorePos+=1;
  }
}
