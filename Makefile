CC = gcc
CFLAGS = -pthread -O3 -Wall -Wextra --pedantic
FILES = $(shell git ls-files '*.c' '*.h' LEIAME Slides.pdf)

ep1: ep1.c state_machine.o amqp_message.o log.o util.o queue_pool.o queue.o hardcoded_values.o round_robin.o

%.o: %.c %.h config.h connection_state.h
	$(CC) $(CFLAGS) -c -o $@ $<

ep1-lucas_seiki_oshiro.tar.gz:
	mkdir ep1-lucas_seiki_oshiro && \
	cp $(FILES) ep1-lucas_seiki_oshiro/ && \
	tar czf $@ ep1-lucas_seiki_oshiro && \
	rm -rf ep1-lucas_seiki_oshiro	

.PHONY: clean
clean:
	rm -f debug ep1 *.o *.tar.gz


