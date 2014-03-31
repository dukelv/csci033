CC = gcc
CFLAGS = -std=c99 -Wall -g -O2 -D_BSD_SOURCE -Werror -lncurses
EXECS = memtop

.PHONY: all
all: $(EXECS)

.PHONY: debug
debug: display_dbg.o memtop.o support.o
	$(CC) $(CFLAGS) display_dbg.o memtop.o support.o -o memtop

display_dbg.o: display.c display.h
	$(CC) $(CFLAGS) -D DEBUG_FLAG -c display.c -o display_dbg.o

memtop: display.o memtop.o support.o
	$(CC) $(CFLAGS) display.o memtop.o support.o -o memtop

memtop.o: memtop.c
	$(CC) $(CFLAGS) -c memtop.c -o memtop.o

support.o: support.c support.h
	$(CC) $(CFLAGS) -c support.c -o support.o

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c -o display.o

.PHONY: clean
clean:
	rm -rvf *.o $(EXECS)
