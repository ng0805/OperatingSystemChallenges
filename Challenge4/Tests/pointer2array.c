#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
 #include <unistd.h>
struct page{

  int addr;
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


void main(){

  printf("pg size = %d\n", getpagesize());
  
  struct page *ar[3], ab, **ac;

  struct tid_tls_pair ttp, ttp2;

  ttp.tid = (pthread_t) 6;

  ttp.tls = (TLS*) malloc(sizeof(TLS));

  ttp.tls -> tid = (pthread_t) 5;

  ttp.tls -> size = 10;

  ttp.tls -> page_num = 3;

  ttp.tls -> pages = (struct page**) malloc(10*sizeof(struct page*));

  for(int i = 0; i < 2;i++){

    struct page *pg = malloc(sizeof(struct page));

    pg -> addr = i;

    pg -> refcount = i+69;

    ttp.tls -> pages[i] = pg;
  }

  printf("%d\n", ttp.tls-> pages[0] -> addr);

  printf("%d\n", ttp.tls -> pages[1] -> addr);

  ttp2.tid = (pthread_t) 7;

  ttp2.tls = (TLS*) malloc(sizeof(TLS));

  ttp2.tls -> tid = (pthread_t) 6;

  ttp2.tls -> size = 11;

  ttp2.tls -> page_num = 4;

  ttp2.tls -> pages = ttp.tls -> pages;

  ttp2.tls -> pages[0] -> addr = 15;
  ttp2.tls -> pages[1] -> addr = 30;

  printf("2[0] = %d\n", ttp2.tls-> pages[0] -> addr);

  printf("2[1] = %d\n", ttp2.tls -> pages[1] -> addr);

  printf("1[0] = %d\n", ttp.tls-> pages[0] -> addr);

  printf("1[1] = %d\n", ttp.tls -> pages[1] -> addr);
}
