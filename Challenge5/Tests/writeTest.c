#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){

  char *write = "My name is Nick", *read = malloc(sizeof(char) * 25), a = 87;

  void *buf;

  buf = (void*) write;

  read = (char*) buf;

  printf("read = %s, read[0] = %c, a = %c, strlen = %ld\n", read, read[0], a, strlen(read));

  char notFull[15] = "Camelot";

  notFull[7] = 129;
  
  printf("notFull = %s, strlen = %ld\n", notFull, strlen(notFull));

  free(read);
}
