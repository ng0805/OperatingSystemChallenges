override CFLAGS := -Wall -Werror -std=gnu99 -pedantic -O0 -g -pthread $(CFLAGS)
override LDLIBS := -pthread $(LDLIBS)


testTLS: tls.o testTLS.o
	$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

multiCloneTest: multiCloneTest.o tls.o
	$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

tls.o: tls.c tls.h

testTLS.o: testTLS.c tls.c

multiCloneTest.o: multiCloneTest.c tls.c

test_files=testTLS

multi_test_files=multiCloneTest

.PHONY: clean

single: $(test_files)
	bash run_tests.sh $(test_files)

multi: $(multi_test_files)
	bash run_tests.sh $(multi_test_files)
clean:
	rm -f *.o $(test_files) *~
	rm -f *.o $(multi_test_files) *~
