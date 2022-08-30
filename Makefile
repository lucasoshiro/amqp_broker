CC = gcc
CFLAGS = -O3 -Wall --pedantic

ep1: ep1.c state_machine.o

debug: debug.c queue_pool.o queue.o

.PHONY: clean
clean:
	rm ep1 *.o
