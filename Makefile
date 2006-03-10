CFLAGS=-Wall -W -g -O0

all: s

s: stackdump.h

clean:
	/bin/rm -f s

.PHONY: all clean
