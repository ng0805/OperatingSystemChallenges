#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>

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

int write(char *src, int off, int ps, int pageNum, int numOfPages, unsigned long int startWrite, unsigned long int endPage){

  int strTrack = 0, count = 0, len = strlen(src);

  unsigned long int addrTracker;
  
  if(startWrite + len > endPage){

    printf("longer\n");

    do{

      addrTracker = startWrite + count;

      if(addrTracker > endPage){

	pageNum++;

	if(pageNum >= numOfPages){

	  printf("write out of bounds\n");

	  return -1;
	}
	else{

	  count = 0;

	  startWrite = tls.pages[pageNum] -> addr;

	  endPage = tls.pages[pageNum] -> addr + ps;

	  addrTracker = startWrite;
	}
      }

      memcpy((void*) addrTracker, &src[strTrack], 1);

      count++;

      strTrack++;

    } while(strTrack < len);
  }

  else{

    memcpy((void*) startWrite, src, len);
  }
}

int read(char *dest, int off, int ps, int pageNum, int numOfPages, int len, unsigned long int startRead, unsigned long int endPage){

  int strTrack = 0, count = 0;

  unsigned long int addrTracker;

  printf("len = %d\n", len);
  
  if(startRead + len > endPage){
  
  do{

    addrTracker = startRead + count; //keep track of current byte position in the address

    if(addrTracker > endPage){ //if at the end of page

      pageNum++; //move to next page

      if(pageNum >= numOfPages){ //if at max page and need to move to next page

	printf("read out of bounds\n");

	return -1;
      }

      else{

	count = 0; //reset count to start at beginning byte of next page

	startRead = tls.pages[pageNum] -> addr; //get new start address for next page

	endPage = tls.pages[pageNum] -> addr + ps; //get new end address for next page

	addrTracker = startRead;
      }
    }

    memcpy(&dest[strTrack], (void*) addrTracker, 1);
      
    count++;

    strTrack++;

  }while(strTrack < len);

  }
  else{

    memcpy(dest, (void*) startRead, len);
  }
}

int main()
{
  tls.pages = malloc(sizeof(struct page*)*2);

  tls.pages[0] = malloc(sizeof(struct page));

  tls.pages[1] = malloc(sizeof(struct page));

  tls.pages[0] -> addr = (unsigned long int) mmap(0, 10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);

  tls.pages[1] -> addr = (unsigned long int) mmap(0, 10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
  
  char *src = "I am Nick ab", *dest = malloc(sizeof(char)*strlen(src));
  
  int off = 4, len = strlen(src), ps = 10, pageNum = off/ps, numOfPages = 2, remoff = off - (pageNum*ps);

  unsigned long int addrTracker, startRW, endPage;

  startRW = tls.pages[pageNum] -> addr + off;

  endPage = tls.pages[pageNum] -> addr + ps;
  
  if(write(src, remoff, ps, pageNum, numOfPages, startRW, endPage) == -1){

    return -1;
  }

  if(read(dest, remoff, ps, pageNum, numOfPages, len, startRW, endPage) == -1){

    return -1;
  }

  printf("dest = %s\n", dest);
  
  //memcpy(&dest0[0], &src[0], 1);

  //printf("dest0[0] = %c\n", dest0[0]);

  printf("done\n");
  
  return 0;
}
