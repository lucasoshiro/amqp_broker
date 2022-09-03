CC = gcc
CFLAGS = -O3 -Wall --pedantic -g

ep1: ep1.c state_machine.o amqp_message.o log.o util.o

debug: debug.c queue_pool.o queue.o util.o

.PHONY: clean
clean:
	rm ep1 *.o
