CC=gcc
CFLAGS=-march=native -g3 -O0 -std=gnu11 -pedantic -Wall -Wextra -Wfloat-equal -Wwrite-strings -Wstrict-prototypes -Wundef -Wshadow

default: build

build: clean
	$(CC) $(CFLAGS) -lcurl -ljson-c -o aursearch main.c util.c

clean:
	rm -rf aursearch
