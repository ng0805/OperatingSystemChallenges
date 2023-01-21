#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#define PAGE_SIZE getpagesize()

struct page
{
  long unsigned int addr;
  int ref_count;
};

typedef struct thread_local_storage
{
  unsigned int size;      /* size in bytes                    */
  unsigned int page_num;  /* number of pages                  */
  struct page **pages;    /* array of pointers to pages       */
} TLS;

TLS tls;

static void sfHandle(int sig, siginfo_t *si, void *context){

  unsigned long int sfPage, indexPage, startAddr;

  sfPage = (unsigned long int) si -> si_addr & (unsigned long int) ~(PAGE_SIZE - 1);

  for(int i = 0; i < 2; i++){

    startAddr = tls.pages[i] -> addr;
    
    indexPage = startAddr & (unsigned long int) ~(PAGE_SIZE - 1);
  
    printf("sfPage =     %lu\n", sfPage);

    printf("indexPage =  %lu\n", indexPage);

    if(sfPage == indexPage){

      printf("error accessing protected memory on page %d\n", i);

      pthread_exit(NULL);
    }

    else{

      printf("page %d not accessed illegaly\n", i);
    }
  }
  
  signal(SIGSEGV, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  raise(sig);
}


void main(){

  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_SIGINFO;

  action.sa_sigaction = sfHandle;

  sigaction(SIGSEGV, &action, NULL);

  sigaction(SIGBUS, &action, NULL);
  
  char *src = "I am Nick ab", *dest = malloc(sizeof(char)*strlen(src));

  char *src2 = "I like trains", *dest2 = malloc(sizeof(char)*strlen(src2));
  
  int len = strlen(src);

  unsigned long int startAddr, off;
  
  tls.pages = malloc(sizeof(struct page*)*2);

  tls.pages[0] = malloc(sizeof(struct page));

  tls.pages[1] = malloc(sizeof(struct page));

  tls.pages[0] -> addr = (unsigned long int) mmap(0, 10, PROT_NONE, MAP_ANON | MAP_PRIVATE, 0, 0);

  tls.pages[1] -> addr = (unsigned long int) mmap(0, 10, PROT_NONE, MAP_ANON | MAP_PRIVATE, 0, 0);

  startAddr = tls.pages[0] -> addr;

  printf("write 1 start\n");
  
  mprotect((void*) startAddr, len, PROT_WRITE);
  
  memcpy((void*) startAddr, src, len);
  
  memcpy(dest, (void*) startAddr, len);
  
  printf("dest = %s\n", dest);

  printf("write 2 start\n");

  startAddr = tls.pages[1] -> addr;

  off = startAddr + 1000;
  
  printf("off = %ld\n", off);
  
  memcpy((void*) off, src2, len);

  memcpy(dest2, (void*) off, len);
  
  /*printf("raising normal segfaul\n");

  raise(SIGBUS);*/
  
}
