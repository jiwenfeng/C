CC=gcc
CFLAGS=-g -Wall
SRC=$(wildcard *.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
TARGET=$(subst .o, , $(OBJ))
LIB=-lpthread

all:$(TARGET)
	@echo "done"

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $< -o $@ $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(TARGET)

