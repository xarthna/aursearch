CC=gcc
CFLAGS=-march=native -std=c11 -pedantic -Wall -Wextra -Wfloat-equal -Wwrite-strings -Wstrict-prototypes -Wundef -Wshadow
LDLIBS=-lcurl -ljson-c
SOURCE_FILES=src/*.c

ifdef DEBUG
	CFLAGS+=-g3 -O0
else
	CFLAGS+=-O2
endif

CFLAGS+=-o bin/aursearch

default: build

build: clean
	$(CC) $(CFLAGS) $(LDLIBS) $(SOURCE_FILES)

clean:
	rm -rf bin/aursearch

.PHONY: clean
