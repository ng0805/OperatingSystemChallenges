# Challenge_4

## Code Explanation
tls.c acts as an extension to the pthread.h library allowing for the creation of a local memory storage for individual threads.

## Functionalities
### Storage
Each tls is stored in the ttpArray structure who's index is created or found using the findIndex local function using the thread's id. 

### Create, Clone, Destroy
tls_create allows for the creation of a tls with the minimum number of pages to fit the specified number of bytes ie. if 4096 bytes are inputed 1 page will be created, but if 4097 bytes are inputed 2 pages will be created.

tls_clone allows a thread to create an exact copy of the tls of another specified thread.

tls_destroy should be called whenever a tls is no longer needed and will iterate througha tls, wiping any page of memory not currently cloned.

Each function will return a -1 upon failure, and a 0 upon success

### Read and Write
tls_read/write allow a string to be read from or written to a tls given a specified offset, length, and string. The functions will allow for read/writng on a page boundary using the boundaryRead/Write functions, in which case the process will continue on the next pages, but will not allow read/write on the edge of a tls.

Each function returns a -1 upon failure and a 0 upon success.

### Misreads/writes
The code implements a handler for seg faults and busses that, when triggered, will scan all available pages and identify if the fault occured trying to access a blocked tls. If this is the case, the function will simply call pthread_exit and allow the other threads to continue, if not then the function will allow the fault to proceed as normal.

## Notes and Known Code Issues
1.) The code seems to randomly pass Gradescope test 12: Attempt to clone a large TLS many times. I am not sure what causes the failure versus the passing, but it is possible for the code to pass the test.
