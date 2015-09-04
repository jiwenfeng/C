CC=gcc
CFLAGS=-g -Wall
SRC=sniffer_capture.c sniffer_config.c sniffer_buffer.c sniffer_log.c sniffer_module.c sniffer_main.c
LIBS=-lpthread -ldl
OBJS=${SRC:%.c=%.o}

all:libs main depend

libs:
	@$(CC) $(CFLAGS) -MM $(SRC) > .libs
-include .libs

main:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS)

depend:
	cd decode && make

clean:
	rm -rf *.o core* main .libs
	cd decode && make clean
