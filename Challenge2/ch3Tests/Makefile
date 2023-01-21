override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.

# Build the threads.o file
barrierCompile: barrierTest

        barrierThread.o: barrierThread.c ec440threads.h

        barrierTest.o: barrierTest.c ec440threads.h

        barrierTest: barrierTest.o barrierThread.o
		$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

        barrier_test_files=barrierTest

mutexCompile: mutexTest

        mutexThread.o: mutexThread.c ec440threads.h

        mutexTest.o: mutexTest.c ec440threads.h

        mutexTest: mutexTest.o mutexThread.o
		$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

        mutex_test_files=mutexTest

# Run the test programs
mutexRun: $(mutex_test_files)
	bash run_tests.sh $(mutex_test_files)

barrierRun: $(barrier_test_files)
	bash run_tests.sh $(barrier_test_files)

clean:
	rm -f *.o $(mutex_test_files) *~
	rm -f *.o $(barrier_test_files) *~
