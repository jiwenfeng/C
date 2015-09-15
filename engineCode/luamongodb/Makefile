CC= g++

AR= ar rcu
RANLIB= ranlib
RM= rm -f

UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LIBS=-L/usr/local/lib -L/usr/local/lib/lua51/ -L/usr/local/mongo/lib -lmongoclient -lboost_thread-mt -lboost_filesystem -lboost_program_options -llua
CFLAGS= -g -O2 -shared -fpic -I/usr/include/ -I /usr/local/mongo/include/mongo/ -I /usr/local/mongo/include/ -I /usr/include/mongo/
else
LIBS=-L/usr/local/lib -L/usr/local/lib/lua51/ -lmongoclient -lboost_thread -lboost_filesystem -lboost_program_options -llua
CFLAGS= -g -O2 -shared -fpic -I/usr/local/include/ -I /usr/local/include/mongo/ -I /usr/local/include/lua51
endif

OUTLIB=mongo.so

LDFLAGS= $(LIBS)

OBJS = main.o mongo_bsontypes.o mongo_connection.o mongo_cursor.o mongo_gridfile.o mongo_gridfs.o mongo_gridfschunk.o mongo_query.o utils.o

all: luamongo

clean:
	$(RM) $(OBJS) $(OUTLIB) mongo.a

old: CFLAGS := $(CFLAGS) -DMONGO_PRE_1_5
old: luamongo 

luamongo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTLIB) $(LDFLAGS)
	ar cr mongo.a $(OBJS)
	ranlib mongo.a

echo:
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "LDFLAGS = $(LDFLAGS)"

main.o: main.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_connection.o: mongo_connection.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_cursor.o: mongo_cursor.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_gridfile.o: mongo_gridfile.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_gridfs.o: mongo_gridfs.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_gridfschunk.o: mongo_gridfschunk.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_query.o: mongo_query.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
mongo_bsontypes.o: mongo_bsontypes.cpp common.h
	$(CC) -c -o $@ $< $(CFLAGS)
utils.o: utils.cpp common.h utils.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all 
