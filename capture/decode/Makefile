CC=g++
CPPCFLAGS=-g -Wall
LIB_INC=-L/usr/local/lib/
LIBS=-lprotobuf -lpthread -ldl

SRC=decode_main.cpp decode_log.cpp

decode.so:$(SRC)
	$(CC) $(CPPCFLAGS) $(LIB_INC) -shared -fpic $(SRC) -o $@ $(LIBS)

clean:
	rm -rf core* *.so main
