#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct testStruct{
  int a;
  char *b;
  bool c;
};

void main(){

  struct testStruct testArray[10];

  struct testStruct empty = {0, "x", false};
  
  testArray[0].a = 10;
  testArray[0].b = "b";
  testArray[0].c = true;
  
  printf("a = %d, b = %s, c = %d\n", testArray[0].a, testArray[0].b, testArray[0].c);

  memset(testArray, 0, sizeof(testArray));

  printf("a = %d, b = %s, c = %d\n", testArray[0].a, testArray[0].b, testArray[0].c);

  if(testArray[9].b == NULL){

    printf("null\n");
  }
}
