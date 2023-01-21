#include <stdio.h>
#include <stdlib.h>

void main(){

  for(int i = 0; i < 6; i++){

    printf("%ld\n", rand() % 4294967295);
  }
}
