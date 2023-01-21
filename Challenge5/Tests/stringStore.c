#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "../disk.h"
#include "../disk.c"

struct dir_entry {
  bool isUsed;
  uint16_t inodeNum;
  char name[15];
};

int main(){

  struct dir_entry arr[2], cpy[2];
  
  arr[0].isUsed = true;
  arr[0].inodeNum = 7;
  strcpy(arr[0].name, "Fart");

  arr[1].isUsed = false;
  arr[1].inodeNum = 8;
  strcpy(arr[1].name,"Cheese");
  
  void *writeBuf = malloc(BLOCK_SIZE), *readBuf = malloc(BLOCK_SIZE);

  unsigned long int addr = (unsigned long int) writeBuf, raddr = (unsigned long int) readBuf;
  
  memcpy(writeBuf, arr, sizeof(arr));

  if(open_disk("String Write") == -1){

    perror("Error opening disk\n");

    return -1;
  }
  
  if(block_write(0, writeBuf) == -1){ // All metadata written to block 0

    perror("Error writing to disk");

    return -1;
  }
  
  if(block_read(0, readBuf) == -1){ // All metadata written to block 0

    perror("Error writing to disk");

    return -1;
  }

  memcpy(cpy, (void*) raddr, sizeof(cpy));

  printf("dqname = %s\n", cpy[0].name);

  if(!strcmp(cpy[0].name, "Fart")){

    printf("success\n");
  }
  
  memset(arr, 0, sizeof(arr));
  
  if(arr[0].name[0] == 0){

    printf("works\n");
  }
  else{

    printf("arr[0].name = %s, %d\n", arr[0].name, (int) arr[0].name[0]);
  }
  
}
