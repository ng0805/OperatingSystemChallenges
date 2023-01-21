#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

struct page
{
  unsigned long int addr;
  int ref_count;
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

struct tid_tls_pair ttp;

void assign(){

  ttp.tls = malloc(sizeof(TLS));

  ttp.tls -> pages = malloc(sizeof(struct page*)*10);

  struct page *pg = malloc(sizeof(struct page));

  pg -> addr = (unsigned long int) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);

  pg -> ref_count = 1;

  ttp.tls -> pages[0] = pg;
}

void set(){

  char *copiee = "I am Nick";

  unsigned long int ad = ttp.tls -> pages[0] -> addr + 2000;
  
  memcpy((int*) ad, copiee, strlen(copiee));
}


void main(){

  assign();

  char *copier, *copiee = "I am Nick", *dest, *src = "U am Nick";
  
  set();

  unsigned long int ab = ttp.tls -> pages[0] -> addr + 2000;
  
  copier = malloc(sizeof(char)*strlen(copiee));
 
  memcpy(copier, (int*) ab, strlen(copiee));
  
  //memcpy(copier, copiee, strlen(copiee));
 
  printf("length = %ld, copier = %s\n", strlen(copiee), copier);
}
