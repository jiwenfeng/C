//精度为一秒的心跳控制器。
#ifndef __CALLOUT_H_
#define __CALLOUT_H_
#include "event.h"
#include <stdlib.h>
#include <assert.h>
#include <lua.hpp>

#define ONCE 1
#define PERSIST 2
const extern luaL_reg callout[];
typedef void (*callcb)(void *);
typedef struct _evdata
{
	struct _evdata* next;
	struct event ev;
	callcb cb;
	void * arg;
	struct timeval tv;
	unsigned int handler;
}evdata;
int get_total_ev();
int get_free_ev();
unsigned int call_once(callcb cb, void * arg, float timeout);
unsigned int call_multi(callcb cb, void * arg, float timeout);
bool rm_call(unsigned int handler, void * *arg); //会回传arg信息.
void init_callout();
#endif//__CALLOUT_H_
