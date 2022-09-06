CC = gcc
CFLAGS = -lpthread -O3 -Wall --pedantic -g

ep1: ep1.c state_machine.o amqp_message.o log.o util.o shared.o queue_pool.o queue.o

debug: debug.c queue_pool.o queue.o util.o

.PHONY: clean
clean:
	rm debug ep1 *.o
