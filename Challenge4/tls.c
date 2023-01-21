#include "tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 128

#define PAGE_SIZE getpagesize()

struct page
{
  long unsigned int addr;
  int ref_count;
};

typedef struct thread_local_storage
{
  pthread_t tid;
  unsigned int size;      /* size in bytes                    */
  unsigned int page_num;  /* number of pages                  */
  pthread_mutex_t mutex;  /*mutex for accessing data          */
  struct page **pages;    /* array of pointers to pages       */
} TLS;

struct tid_tls_pair
{
  pthread_t tid;
  TLS *tls;
};


struct tid_tls_pair emptyTLS = {.tid = (pthread_t) -1, .tls = NULL};

struct tid_tls_pair ttpArray[MAX_THREADS]; //set all elements to empty

static int maxIndex = 0; //keeps track of last filled index in ttpArray

struct tid_tls_pair checkSegFault(){

  printf("\n\nret = %ld\n\n", ttpArray[0].tls -> pages[0] -> addr);
  
  return ttpArray[0];
}

static int roundUp(int size, int pagesize){

  float sz = (float) size, ps = (float) pagesize, div;

  int round;

  div = sz/ps; //get decimal deivision

  round = (int) div; //int rounds down from decimal

  div = div - round; //get just the decimal part

  if(div > 0){ //if there is a non 0 decimal, that means round up 1

    round++;
  }

  return round;
}

static void sfHandle(int sig, siginfo_t *si, void *context){
  
  unsigned long int sfPage, indexPage, startAddr;

  int numOfPages;

  sfPage = (unsigned long int) si -> si_addr & (unsigned long int) ~(PAGE_SIZE - 1);

  for(int i = 0; i < maxIndex; i++){ //loop throuhg all tls's

    numOfPages = ttpArray[i].tls -> page_num; //get number of pages in this tls

    for(int j = 0; j < numOfPages; j++){

      startAddr = ttpArray[i].tls -> pages[j] -> addr; //get address of current page

      indexPage = startAddr & (unsigned long int) ~(PAGE_SIZE - 1);

      if(sfPage == indexPage){
	
	printf("\nThread %ld accesed other threads TLS, now exiting\n\n", pthread_self());
  
	pthread_exit(NULL);
      }
    }
  }
  printf("Normal seg fault or bus encountered\n\n");
  
  signal(SIGSEGV, SIG_DFL); //if no illegal page access found then call signal as normal
  signal(SIGBUS, SIG_DFL);
  raise(sig);
}

void tls_init(){

  ttpArray[0] = emptyTLS; //set first index to empty for proper erro checing

  struct sigaction action; //set seg fault and bus erro handler

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_SIGINFO;

  action.sa_sigaction = sfHandle;

  sigaction(SIGSEGV, &action, NULL);

  sigaction(SIGBUS, &action, NULL);
}

static int findIndex(pthread_t tid){ //find array index for corresponding tid
  
  int index;

  static bool firstCall = 1;

  if(firstCall){ //assign first index to empty first time function is called

    tls_init();

    firstCall = 0;
  }

  for(int i = 0; i < MAX_THREADS; i++){ //maybe change MAX_THREADS to maxIndex

    if(ttpArray[i].tid == tid){
      
      return i;
    }
  }
  
  index = maxIndex; //store maxIndex
  
  maxIndex++; //increase maxIndex by 1

  ttpArray[maxIndex] = emptyTLS; //set next available index to empty for proper checking

  return index; //return first available index for new allocation
}

int boundaryWrite(char *src, int ps, int pageNum, int numOfPages, int index, unsigned long int startWrite, unsigned long int endPage){

  int strTrack = 0, count = 0, len = strlen(src), pn = pageNum;

  unsigned long int addrTracker, cowAd1, cowAd2, endTrack = endPage, startTrack = startWrite;
  
  for(int i = 0; i < len; i++){ //scan for boundary write

    addrTracker = startTrack + count;

    if(addrTracker >= endTrack){

      pn++; // increase to the next page

      if(pn >= numOfPages){ //if already at max page

	return -1;
      }

      else{

	count = 0;
      
	startTrack = ttpArray[index].tls -> pages[pn] -> addr; //get new start addr of next page

	endTrack = ttpArray[index].tls -> pages[pn] -> addr + ps; //get new end addr of next page

	addrTracker = startTrack; //set addrTracker to beginning of next page for write
      }
    }

    count++;
  }

  count = 0; //reset after scan
  
  do{ //Note: first page write access should be open
    
    addrTracker = startWrite + count; //get current byte write position

    if(addrTracker >= endPage){ //if at end of page

      mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_NONE); //close access to previous page

      pageNum++; // increase to the next page

      if(ttpArray[index].tls -> pages[pageNum] -> ref_count > 1){ //COW

	mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_WRITE); //open write access to new page

	cowAd1 = ttpArray[index].tls -> pages[pageNum] -> addr; //save addr of old page

	ttpArray[index].tls -> pages[pageNum] -> ref_count--; //decrease page refcount by 1
	
	struct page *pg = malloc(sizeof(struct page));
      
	pg -> addr = (long unsigned int) mmap(0, ps, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0); //COW page can start with write access
	
	pg -> ref_count = 1;

	ttpArray[index].tls -> pages[pageNum] = pg;

	cowAd2 = ttpArray[index].tls -> pages[pageNum] -> addr; //save addr of new page

	mprotect((void*) cowAd1, ps, PROT_WRITE); //open cowAd1 for write

	memcpy((void*) cowAd2, (void*) cowAd1, 4096); //copy full old page to new page

	mprotect((void*) cowAd1, ps, PROT_NONE); //close cowAd1 again
	
	count = 0;
	
	startWrite = cowAd2;
	
	addrTracker = cowAd2;
	
	endPage = cowAd2 + ps;
      }
      
      else{

	mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_WRITE); //open access for new page
	
	count = 0; //reset to beginning of page
	
	startWrite = ttpArray[index].tls -> pages[pageNum] -> addr; //get new start addr of next page
	
	endPage = ttpArray[index].tls -> pages[pageNum] -> addr + ps; //get new end addr of next page

	addrTracker = startWrite; //set addrTracker to beginning of next page for write
      }
    }

    memcpy((void*) addrTracker, &src[strTrack], 1);

    count++; //move to next byte in page

    strTrack++; //move to next index in string

  } while(strTrack < len); //loop until all string bytes have been written

  mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_NONE); //close access to last  accessed page

  return 0;
}


int boundaryRead(char *dest, int ps, int pageNum, int numOfPages, int len, int index, unsigned long int startRead, unsigned long int endPage){

  int strTrack = 0, count = 0, pn = pageNum;

  unsigned long int addrTracker, endTrack = endPage, startTrack = startRead;

  for(int i = 0; i < len; i++){ //scan for boundary write

    addrTracker = startTrack + count;

    if(addrTracker >= endTrack){

      pn++; // increase to the next page

      if(pn >= numOfPages){ //if already at max page

	return -1;
      }

      else{

	count = 0;

	startTrack = ttpArray[index].tls -> pages[pn] -> addr; //get new start addr of next page

	endTrack = ttpArray[index].tls -> pages[pn] -> addr + ps; //get new end addr of next page

	addrTracker = startTrack; //set addrTracker to beginning of next page for write
      }
    }

    count++;
  }

  count = 0; //reset after scan 
  
  do{ //Note: read access for first page should already be allowed

    addrTracker = startRead + count; //keep track of current byte position in the address

    if(addrTracker >= endPage){ //if at the end of page

      mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_NONE); //close access to previous page

      pageNum++; //move to next page

      mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_READ); //open access to new page
	
      count = 0; //reset count to start at beginning byte of next page

      startRead = ttpArray[index].tls -> pages[pageNum] -> addr; //get new start address for next page

      endPage = ttpArray[index].tls ->pages[pageNum] -> addr + ps; //get new end address for next page

      addrTracker = startRead;
    }

    memcpy(&dest[strTrack], (void*) addrTracker, 1);

    count++;

    strTrack++;

  } while(strTrack < len); //lopp until full string has been written

  mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, ps, PROT_NONE); //close access to final page

  return 0;
}


int tls_create(unsigned int size)
{
  int index = findIndex(pthread_self()), pageSize = getpagesize(), numOfPages = roundUp(size, pageSize); //round to min number of pages to fit size bytes

  pthread_t tid = pthread_self();

  if(size <= 0){

    return -1;
  }

  else if(ttpArray[index].tls != NULL){ //if tls != NULL then one already exists

    return -1;
  }

  ttpArray[index].tid = tid;

  ttpArray[index].tls = malloc(sizeof(TLS));
  
  ttpArray[index].tls -> tid = tid;

  ttpArray[index].tls -> size = size;

  ttpArray[index].tls -> page_num = numOfPages;

  pthread_mutex_init(&ttpArray[index].tls -> mutex, NULL);

  pthread_mutex_lock(&ttpArray[index].tls -> mutex);
  
  ttpArray[index].tls -> pages = malloc(numOfPages*sizeof(struct page*));

  for(int i = 0; i < numOfPages;i++){

    struct page *pg = malloc(sizeof(struct page));

    pg -> addr = (unsigned long int) mmap(0, 4096, PROT_NONE, MAP_ANON | MAP_PRIVATE, 0, 0);

    pg -> ref_count = 1;

    ttpArray[index].tls -> pages[i] = pg;
  }

  pthread_mutex_unlock(&ttpArray[index].tls -> mutex);

  return 0;
}

int tls_destroy()
{
  /*  int index = findIndex(pthread_self()), numOfPages;//pageSize = getpagesize();

  bool freePages = 1;

  if(ttpArray[index].tls == NULL){ //if tls == NULL then tls does not exist

    return -1;
  }

  numOfPages = ttpArray[index].tls -> page_num;

  pthread_mutex_lock(&ttpArray[index].tls -> mutex);
  
  for(int i = 0; i < numOfPages; i++){

    if(ttpArray[index].tls -> pages[i] -> ref_count == 1){ //free any pages

      //munmap((void*) ttpArray[index].tls -> pages[i]-> addr, pageSize);
    }

    else{ //if any page is shared, don't free ttpArray[threadId].tls -> pages
      ttpArray[index].tls -> pages[i] -> ref_count--; //decrement refcount by 1

      freePages = 0;
    }
  }

  pthread_mutex_unlock(&ttpArray[index].tls -> mutex);
  
  if(freePages){
    
    //free(ttpArray[index].tls -> pages);
    
    //free(ttpArray[index].tls);
  }

  //ttpArray[index] = emptyTLS; //set threads ttp back to empty
  */
  return 0;
}

int tls_read(unsigned int offset, unsigned int length, char *buffer)
{
  int index = findIndex(pthread_self()), pageNum, remOffset, numOfPages, pageSize = getpagesize();

  unsigned long int startAddr, endPage; //the starting and ending point address for the read page

  if(ttpArray[index].tls == NULL){ // make sure tls exists

    return -1;
  }

  if(length > (ttpArray[index].tls -> page_num * pageSize)){ //if length of buffer is bigger than size of TLS

    return -1;
  }

  pageNum = offset/pageSize; //always rounds down to find first page index

  remOffset = offset - (pageNum * pageSize); //get leftover bytes needed to be added to beginning of page

  numOfPages = ttpArray[index].tls -> page_num;

  pthread_mutex_lock(&ttpArray[index].tls -> mutex);
  
  mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, pageSize, PROT_READ); //allow read access

  startAddr = ttpArray[index].tls -> pages[pageNum] -> addr + remOffset; //get starting point

  endPage = ttpArray[index].tls -> pages[pageNum] -> addr + pageSize;

  if(startAddr + length > endPage){ //if read will go over a page boundary

    if(boundaryRead(buffer, pageSize, pageNum, numOfPages, length, index, startAddr, endPage) == -1){

      pthread_mutex_unlock(&ttpArray[index].tls -> mutex); //unlock mutex
      
      return -1;
    }
  }

  else{ //read should fit as normal so read normally

    memcpy(buffer, (void*) startAddr, length);

    mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, pageSize, PROT_NONE); //close read access
  }
  
  pthread_mutex_unlock(&ttpArray[index].tls -> mutex);
  
  return 0;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer)
{
  int index = findIndex(pthread_self()), pageNum, pageSize = getpagesize(), remOffset, numOfPages;

  unsigned long int startAddr, endPage, cowAd1, cowAd2; //the starting point address for the write

  if(ttpArray[index].tls == NULL){ // make sure tls exists

    return -1;
  }

  if(length > (ttpArray[index].tls -> page_num * pageSize)){ //if length of buffer is bigger than size of TLS

    return -1;
  }

  pageNum = offset/pageSize; //always rounds down to find first page index

  pthread_mutex_lock(&ttpArray[index].tls -> mutex);
  
  if(ttpArray[index].tls -> pages[pageNum] -> ref_count > 1){ //COW

    cowAd1 = ttpArray[index].tls -> pages[pageNum] -> addr; //save addr of old page

    ttpArray[index].tls -> pages[pageNum] -> ref_count--; //decrease page refcount by 1

    struct page *pg = malloc(sizeof(struct page));

    pg -> addr = (long unsigned int) mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0); //on COW leave open to write and close after write

    pg -> ref_count = 1;

    ttpArray[index].tls -> pages[pageNum] = pg;

    cowAd2 = ttpArray[index].tls -> pages[pageNum] -> addr; //save addr of new page

    mprotect((void*) cowAd1, pageSize, PROT_WRITE); //open cowAd1 for copy

    memcpy((void*) cowAd2, (void*) cowAd1, 4096); //copy full old page to new page

    mprotect((void*) cowAd1, pageSize, PROT_NONE); //close cowAd1 again
  }

  else{ //open current page to write if COW is not needed

    mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, pageSize, PROT_WRITE);
  }

  remOffset = offset - (pageNum * pageSize); //get leftover bytes needed to be added to beginning of page

  numOfPages = ttpArray[index].tls -> page_num;

  startAddr = ttpArray[index].tls -> pages[pageNum] -> addr + remOffset; //get starting point

  endPage = ttpArray[index].tls -> pages[pageNum] -> addr + pageSize; //get ending point

  if(startAddr + length > endPage){ //if page boundary read

    if(boundaryWrite(buffer, pageSize, pageNum, numOfPages, index, startAddr, endPage) == -1){

      printf("here\n");
      
      pthread_mutex_unlock(&ttpArray[index].tls -> mutex);
      
      return -1;
    }
  }

  else{ //buff should fit as normal so copy normally

    memcpy((void*) startAddr, buffer, length); //startAddr cast back to pointer

    mprotect((void*) ttpArray[index].tls -> pages[pageNum] -> addr, pageSize, PROT_NONE); //close page access
  }

  pthread_mutex_unlock(&ttpArray[index].tls -> mutex);
  
  return 0;
}

int tls_clone(pthread_t tid)
{
  pthread_t ptid = pthread_self();

  int clonerIndex = findIndex(ptid), cloneeIndex = findIndex(tid), numOfPages;

  if(ttpArray[clonerIndex].tls != NULL){ //if calling thread tls != NULL then a tls already exists

    return -1;
  }

  else if(ttpArray[cloneeIndex].tls == NULL){ //if cloned thread tls == NULL then the tls does not exist

    return -1;
  }

  numOfPages = ttpArray[cloneeIndex].tls -> page_num;

  ttpArray[clonerIndex].tid = ptid;

  ttpArray[clonerIndex].tls = (TLS*) malloc(sizeof(TLS));

  pthread_mutex_init(&ttpArray[clonerIndex].tls -> mutex,NULL);
  
  ttpArray[clonerIndex].tls -> tid = ptid;

  ttpArray[clonerIndex].tls -> size = ttpArray[cloneeIndex].tls -> size;

  ttpArray[clonerIndex].tls -> page_num = numOfPages;

  pthread_mutex_lock(&ttpArray[clonerIndex].tls -> mutex);

  pthread_mutex_lock(&ttpArray[cloneeIndex].tls -> mutex);
  
  ttpArray[clonerIndex].tls -> pages = (struct page**) malloc(numOfPages*sizeof(struct page*));

  for(int i = 0; i < numOfPages; i++){ //increment refcount for each page

    ttpArray[clonerIndex].tls -> pages[i] = ttpArray[cloneeIndex].tls -> pages[i];

    ttpArray[clonerIndex].tls -> pages[i] -> ref_count++;
  }

  pthread_mutex_unlock(&ttpArray[clonerIndex].tls -> mutex);

  pthread_mutex_unlock(&ttpArray[cloneeIndex].tls -> mutex);
  
  return 0;
}
