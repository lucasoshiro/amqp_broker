CC = gcc
CFLAGS = -O3 -Wall --pedantic

ep1: ep1.c

.PHONY: clean
clean:
	rm ep1
