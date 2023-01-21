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

  int writeFd;
  
  char *writeBuf1 = "My name is Nick", *writeBuf2 = "I cant wait to be done with this";
  
  TEST_ASSERT(make_fs("Test FS") != -1);

  TEST_ASSERT(mount_fs("Test FS") != -1);

  TEST_ASSERT(fs_create("Test File") != -1);
  /*  
  TEST_ASSERT(fs_create("ab") != -1);
  TEST_ASSERT(fs_create("abcd") != -1);
  TEST_ASSERT(fs_create("hijklm") != -1);
  TEST_ASSERT(fs_create("Dummy Name") != -1);
  TEST_ASSERT(fs_create("cheeseburger") != -1);
  TEST_ASSERT(fs_create("Fifteen Letters") != -1);
  TEST_ASSERT(fs_create("Sixteen Letterrs") == -1);
*/  

  writeFd = fs_open("Test File");

  TEST_ASSERT(writeFd != -1);
  
  TEST_ASSERT(fs_write(writeFd, writeBuf1, strlen(writeBuf1)) != -1);

  TEST_ASSERT(fs_write(writeFd, writeBuf2, strlen(writeBuf2)) != -1);

  TEST_ASSERT(fs_close(writeFd) != -1);
  
  TEST_ASSERT(umount_fs("Test FS") != -1);

  printf("Success\n");
}
