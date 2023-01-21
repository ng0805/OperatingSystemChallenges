override CFLAGS := -Wall -Werror -std=gnu99 -pedantic -O0 -g $(CFLAGS)

fileTest: fileTest.o fs.o disk.o
	$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

fileTest.o: fileTest.c fs.c disk.c

makefs: makefs.o fs.o disk.o
	$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

makefs.o: makefs.c fs.c disk.c

fs.o: fs.c fs.h disk.o

disk.o: disk.c disk.h

makeTestFiles=makefs

ft=fileTest

.PHONY: clean

fileSys: $(makeTestFiles)
	bash run_tests.sh $(makeTestFiles)

fileManipulate: $(ft)
	bash run_tests.sh $(ft)

clean:
	rm -f fs.o
