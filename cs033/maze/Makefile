CC = gcc
GEN = generator
SOL = solver
SOL_FULL = solver_full
CFLAGS = -g -Wall -Wextra

GEN_OBJS = generator.c
SOL_OBJS = solver.c

all: solver generator solver_full maze

generator: $(GEN_OBJS)
	$(CC) $(CFLAGS) -o $(GEN) $(GEN_OBJS)

solver: $(SOL_OBJS)
	$(CC) $(CFLAGS) -o $(SOL) $(SOL_OBJS)

solver_full: $(SOL_OBJS)
	$(CC) $(CFLAGS) -o $(SOL_FULL) -DFULL $(SOL_OBJS)

clean:
	rm -f $(GEN) $(SOL) $(SOL_FULL)
