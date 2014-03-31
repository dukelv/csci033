CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wunused
CFLAGS += -g -std=c99 -D_GNU_SOURCE -pthread

OBJECTS = db.o window.o io_functions.o timeout.o
EXECS = interface server

.PHONY: all
all: $(EXECS)

server: io_functions.o window.o timeout.o db.o server.c
	$(CC) $(CFLAGS) $^ -o $@

interface: io_functions.o interface.c
	$(CC) $(CFLAGS) $^ -o $@

db.o: db.h
window.o: window.h io_functions.h
io_functions.o: io_functions.h
timeout.o: timeout.h

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rvf *.o $(EXECS)

