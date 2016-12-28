CC=gcc
CFLAGS=-g -Wall
SRC=$(wildcard *.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
TARGET=$(patsubst %.o, %.out, $(OBJ))
LIBS=-lpthread

all:$(TARGET)
	@echo "done"

%.out:%.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(TARGET)

