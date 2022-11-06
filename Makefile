CC=gcc
CFLAGS=-march=native -g3 -O0 -std=gnu11 -pedantic -Wall -Wextra -Wfloat-equal -Wwrite-strings -Wstrict-prototypes -Wundef -Wshadow

default: build

build: clean
	$(CC) $(CFLAGS) -lcurl -ljson-c -o bin/aursearch src/main.c src/print.c src/request.c

clean:
	rm -rf bin/aursearch
