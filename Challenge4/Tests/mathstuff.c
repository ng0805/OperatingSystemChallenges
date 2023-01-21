#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void main(){

  printf("pg size = %d\n", getpagesize());

  int size = 1, pagesize = 4096, nop;

  float sz = (float) size, ps = (float) pagesize, div, store;

  div = sz/ps;

  printf("div 1 = %f\n", div);
  
  nop = div;

  div = div - nop;

  printf("div 2 = %f\n", div);
  
  if(div > 0){

    nop++;
  }
  
  //nop = (int) ceil((float) size/pagesize);

  //nop = (size+pagesize-1)/pagesize;

  //nop = (size/pagesize) + ((size % pagesize) != 0);
  
  printf("pg size = %d\n", nop);
}
