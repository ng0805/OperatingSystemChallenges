#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../disk.h"
#include "../disk.c"

void tpp(char ***arrtp){

  char *arr[3] = {"name1","name2", "name3" },**arrdp = (char**) malloc(4*sizeof(char*));

  for (int i = 0; i < 3; i++){

    arrdp[i] = arr[i];
  }

  arrdp[3] = NULL;

  printf("arrdp: ");

  for(int i = 0; i < 4; i++){

    printf("%s ", arrdp[i]);
  }

  printf("\n");

  *arrtp = arrdp;
}


int main()
{
  char **arrfill = (char**) malloc(4*sizeof(char*));

  tpp(&arrfill);

  printf("arrfill: ");

  for(int i = 0; i < 4; i++){

    printf("%s ", arrfill[i]);
  }

  printf("\n");

  return 0;
}
