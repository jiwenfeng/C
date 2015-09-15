#include <lua.hpp>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <event.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <signal.h>

extern FILE* Logfile;
const char * Now();

#define _WARN(fmt, ...) fprintf(Logfile, "%s:%d [%s] WARN: "fmt"\n", __FILE__, __LINE__, Now(), ##__VA_ARGS__)
#define _RUNTIME_ERROR(fmt, ...) fprintf(stdout, "%s:%d "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _ERROR(fmt, ...) fprintf(Logfile, "%s:%d [%s] ERROR: "fmt"\n", __FILE__, __LINE__, Now(), ##__VA_ARGS__)


extern lua_State* LState;
