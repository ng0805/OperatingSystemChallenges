# Challenge_2

## Code Explanation
threads.c acts as a simpler definition for the pthread library included through the <pthread.h> library. It includes the pthread_create, pthread_exit, pthread_self, and helper functions. 
### Scheduling
The code runs on a round robin scheduler that will run a thread for 50 ms before jumping to the next available thread
### Mutexes and Barriers
The code employs both mutex and barrier functionalites. A mutex of type pthread_mutex_t can be initialized with pthread_mutex_init and then locked and unlocked with pthread_mutex_lock/unlock to protext critical data regions. A barrier of type pthread_barrier_t can be initialized with pthread_barrier_init making sure to specify a count, and then individual threads can wait on the barrier with pthread_barrier_wait which will block all waiting threads until count is met.
## Code Flow
### Startup
To compile and run the code, the user simply needs to run make and then make check in the terminal. If you want to use other test files outside the test_busy_threads.c make sure to modify the make file.

### Normal Process
After startup, the code will use the test file as a main file for the pthread library. It will create the threads and rotate between them using the scheduler. All threads will automatically
run pthread_exit and once all threads are finished, the program will exit.

## Notes and Known Code Issues
1.) the threads.c code history does show its basic progress but for a more indepth look at the coding history, see the individualThreads.c, icTest.c, simpleScheduling.c, schedTest.c in the experiments folder, mutexThread.c, mutexTest.c, barrierThread.c, and barrierTest.c files in the ch3Tests folder where thread creation, scheduling, mutexes, and barriers were tested.
