#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

uint32_t usedBlocksBitmap[256];

bool setBit(int position, bool x, bool readWrite){ //0 for read, 1 for write

  uint32_t index = position/32, bit = position%32, flag = 1;

  flag = flag << bit; //000...010...000

  if(readWrite){ //if writing bit

    if(x){ //setting bit to 1

      usedBlocksBitmap[index] = usedBlocksBitmap[index] | flag;
    }

    else{
    
      flag = ~flag; ///111...101...111

      usedBlocksBitmap[index] = usedBlocksBitmap[index] & flag;
    }

    return x;
  }
  
  else{ //if reading bit

    return usedBlocksBitmap[index] & flag;
  }
}

void main(){

  int check[10] = {1,24,56,8191,0,1002,2001,2048,5173,7777};

  memset(usedBlocksBitmap, 0, sizeof(usedBlocksBitmap)); //fill ubb with 0's

  int test = 1;

  printf("test = %x, shuift test = %x\n", test, test << 1);
  
  for(int i = 0; i < 8192; i++){

    if(i%2 == 0){

      setBit(i, 1, 1);
    }

    else{

      setBit(i, 0, 1);
    }
  }
  
  for(int i = 0; i < 10; i++){

    bool x = setBit(check[i], 1, 0);

    printf("bit %d: %d, ", check[i], x);
  }

  printf("\n");
}
