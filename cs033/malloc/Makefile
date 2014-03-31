CC = gcc
CFLAGS = -Wall -Wextra -Wunused -O2 -m32 -Werror

OBJS = mdriver.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

all: mdriver mdriver-naive

mdriver: $(OBJS) mm.o
	$(CC) $(CFLAGS) -o mdriver $(OBJS) mm.o
mdriver-naive: $(OBJS) mm-naive.o
	$(CC) $(CFLAGS) -o mdriver-naive $(OBJS) mm-naive.o

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

mm.o: mm.c mm.h memlib.h
mm-naive.o: mm-naive.c mm.h memlib.h

clean:
	rm -f *~ *.o mdriver mdriver-naive
