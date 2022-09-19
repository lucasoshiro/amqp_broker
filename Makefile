CC = gcc
CFLAGS = -pthread -O3 -Wall -Wextra --pedantic

ep1: ep1.c state_machine.o amqp_message.o log.o util.o queue_pool.o queue.o hardcoded_values.o round_robin.o

%.o: %.c %.h config.h connection_state.h
	$(CC) $(CFLAGS) -c -o $@ $<

debug: debug.c queue_pool.o queue.o util.o

.PHONY: clean
clean:
	rm -f debug ep1 *.o
