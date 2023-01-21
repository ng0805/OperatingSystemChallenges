#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct page
{
  unsigned int addr;
  int refcount;
};

typedef struct thread_local_storage
{
  pthread_t tid;
  unsigned int size;      /* size in bytes                    */
  unsigned int page_num;  /* number of pages                  */
  struct page **pages;    /* array of pointers to pages       */
} TLS;

struct tid_tls_pair
{
  pthread_t tid;
  TLS *tls;
};

void main (){

  struct tid_tls_pair empty = {.tid = (pthread_t) -1, .tls = NULL};

  struct tid_tls_pair ttp[10];
  
  if(ttp[0].tls == NULL){

    printf("works\n");
  }

  else{

    printf("%ld\n", ttp[0].tid);
    
    printf("%p\n",ttp[0].tls);
  }

}
