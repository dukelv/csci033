CC = gcc
CFLAGS = -Wall -O2 -pthread -std=c99 -D_GNU_SOURCE
EXTRAFLAGS = #Can be set in the call to make

SEQUENTIAL = sequential
RANDOM = random

OBJECTS = linkedlist.c

EXECS = sequential random

.PHONY: all
all: $(EXECS)

sequential: $(OBJECTS)
	$(CC) $(CFLAGS) $(EXTRAFLAGS) $^ -DSEQUENTIAL -o $@

random: $(OBJECTS)
	$(CC) $(CFLAGS) $(EXTRAFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRAFLAGS) -c $*.c -o $*.o

.PHONY: clean
clean:
	rm -rvf *.o $(EXECS)
