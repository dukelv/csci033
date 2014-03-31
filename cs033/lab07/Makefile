CC = gcc
CFLAGS = -Wall -O2 -g -std=c99

EXECS = tester

.PHONY: all
all: $(EXECS)

tester: string_fns.o hashset.o tester.c
	$(CC) $(CFLAGS) $^ -o $@

string_fns.o: string_fns.c string_fns.h
	$(CC) $(CFLAGS) -c string_fns.c -o string_fns.o

hashset.o: hashset.c hashset.h
	$(CC) $(CFLAGS) -c hashset.c -o hashset.o

.PHONY: clean
clean:
	rm -rvf *.o $(EXECS)
