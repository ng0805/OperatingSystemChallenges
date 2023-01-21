#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../disk.h"
#include "../disk.c"

#define DISK_BLOCKS  8192      /* number of blocks on the disk                */
#define BLOCK_SIZE   4096      /* block size on "disk"                        */

//int writeInt[5], readInt[5];

//char writeChar[5], readChar[5];

int makeDisk(){

  char *disk_name = "TestDisk";
  
  char *writeBuf = malloc(BLOCK_SIZE);

  char *readBuf = malloc(BLOCK_SIZE);
  
  unsigned long int addr = (unsigned long int) writeBuf;
  
  for(int i = 0; i < 5; i++){

    writeBuf[i] = i+1;

    writeBuf[i+5] = (char) i+66;
  }
  
  /*if(make_disk(disk_name) == -1){

    perror("Error making disk\n");

    return -1;
    }*/

  if(open_disk(disk_name) == -1){

    perror("Error opening disk");

    return -1;
  }

  if(block_write(0, writeBuf) == -1){

      perror("Error writing to disk");

      return -1;
  }

  if(block_read(0, readBuf) == -1){

    perror("Error reading from disk");

    return -1;
  }

  for(int i = 0; i < 5; i++){

    printf("%d %c ", readBuf[i], readBuf[i+5]);
  }
    
  printf("\n");

  if(close_disk() == -1){

    perror("Error closing disk");

    return -1;
  }
  
  free(writeBuf);

  free(readBuf);

  return 0;
}

void main(){
  
  if(makeDisk() == -1){

    printf("Error in disk creation\n");

    exit(1);
  }

  else{

    printf("Success!\n");

    exit(0);
  }
}
