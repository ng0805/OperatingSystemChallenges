#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "fs.h"

#define TEST_ASSERT(x) do { \
  if (!(x)) { \
  fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
	  __FILE__, __LINE__, #x); \
  abort(); \
  } \
  } while(0)

int main(){

  int badFd, readFd, fileListIter = 0;;

  char **ls = (char**) malloc(65*sizeof(char*));
  char *readBuf1 = malloc(15), *readBuf2 = malloc(33), *readBuf3 = malloc(15);
  char *writeBuf1 = "My name is Nick", *writeBuf2 = "I cant wait to be done with this";
  
  TEST_ASSERT(mount_fs("Test FS") != -1);

  printf("Mount Success\n");

  TEST_ASSERT(fs_listfiles(&ls) != -1);

  while(ls[fileListIter] != NULL){

    printf("%s ", ls[fileListIter]);

    fileListIter++;
  }

  printf("\n");
  
  printf("List files success\n");
    
  badFd = fs_open("I Dont Exist");
  
  TEST_ASSERT(badFd == -1);

  printf("Bad open Success\n");
  
  readFd = fs_open("Test File");

  TEST_ASSERT(readFd != -1);

  printf("Read open success\n");

  TEST_ASSERT(fs_read(readFd, readBuf1, 15) != -1);

  TEST_ASSERT(strcmp(readBuf1, writeBuf1) == 0);
  
  printf("Read 1 success, readBuf1 = %s\n", readBuf1);

  TEST_ASSERT(fs_read(readFd, readBuf2, 32) != -1);

  TEST_ASSERT(strcmp(readBuf2, writeBuf2) == 0);
  
  printf("Read 2 success, readBuf2 = %s\n", readBuf2);

  TEST_ASSERT(fs_lseek(readFd, 0) != -1);

  printf("Lseek success\n");
  
  TEST_ASSERT(fs_read(readFd, readBuf3, 15) != -1);

  TEST_ASSERT(strcmp(readBuf3, writeBuf1) == 0);

  printf("Read 3 success, readBuf3 = %s\n", readBuf3);
  
  TEST_ASSERT(fs_close(badFd) == -1);

  printf("Bad close success\n");
  
  TEST_ASSERT(fs_close(readFd) != -1);

  printf("Read close success\n");
  
  TEST_ASSERT(umount_fs("Test FS") != -1);

  printf("Unmount Success\n");
  
  free(ls);
}
