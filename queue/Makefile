CFLAG=-Wall -g -O2 -I. -I/usr/local/include/ -I/usr/include/
CC=gcc
OBJ=queue.o
main:$(OBJ)
	$(CC) $(CFLAG) $(OBJ) -o main 

queue.o:queue.c queue.h
	$(CC) $(CFLAG) -c $< 

clean:
	rm -rf *.o main
