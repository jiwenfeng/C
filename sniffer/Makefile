CFLAGS=-g -Wall -O2
CC=gcc
LIB=-ldl -llua -lm -lpthread
SRC=sniffer_main.c sniffer_buffer.c linux/sniffer_op.c sniffer_config.c sniffer_parse.c sniffer_log.c linux/sniffer_thread.c
OBJ=${SRC:%.c=%.o}
all:depend main

depend:$(OBJ)
	@$(CC) -MM $(CFLAGS) $(SRC) $(LIB) > .depend	
-include .depend

main:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIB)

clean:
	rm -rf $(OBJ) main .depend *.log core
