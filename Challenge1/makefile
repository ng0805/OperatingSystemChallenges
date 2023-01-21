override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.

myshell: myshell.o myshell_parser.o
	$(CC) $(LDFLAGS) $+ $(LOADLIBES) $(LDLIBS) -o $@

myshell_parser.o: myshell_parser.c myshell_parser.h

myshell.o: myshell.c myshell_parser.c
