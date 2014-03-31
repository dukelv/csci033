# Assembler
AS = as
AFLAGS = -g

# C Compiler
CC = gcc
CFLAGS = -g -Wall -std=c99

OBJECTS = my_atoi.o

EXECS = atoi

.PHONY: all
all: $(EXECS)

$(OBJECTS): %.o: %.s
	$(AS) $(AFLAGS) $^ -o $@

$(EXECS): %: my_%.o %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rvf $(OBJECTS) $(EXECS)
