CC = gcc
CFLAGS = -Wall -pthread

.PHONY: all clean

all: ACS

ACS: acs.c queue.c
	$(CC) $(CFLAGS) acs.c queue.c -o ACS

clean:
	rm -f ACS *.o