CFLAGS=-g -Wall -O2 -I. -I/usr/local/include/ -I/usr/include/
CC=gcc
LIBS=memory.o

main:$(LIBS)
	$(CC) $(CFLAGS) $(LIBS) -o $@

memory.o:memory.c memory.h
	$(CC) -c -o $@ $< $(CFLAGS) 

clean:
	rm -rf $(LIBS) main core

