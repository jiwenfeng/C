#include <map>
#include "global.hpp"
#include "lmemory.hpp"
#include "netlib.hpp"
#include "luacall.hpp"
#include "callout.hpp"
#include "lstack.hpp"
#include "api.hpp"
#include "lscene.hpp"
#include "msgpackLua.hpp"

lua_State* LState = NULL;
FILE* Logfile = NULL;
size_t gateWayDataLen = 0;
const char* gateWayData = NULL;

const char* EXIT_SCRIPT = "signal_exit";

extern "C" int luaopen_lposix (lua_State *L);
extern "C" int luaopen_mongo(lua_State *L);

//!!not thread safe
const char * Now()
{
	time_t tt = time(NULL);
	struct tm * tmNow = localtime(&tt);

	static char NowString[20];
	strftime(NowString, sizeof(NowString), "%F %T", tmNow);
	return NowString;
}

void gc( void* arg)
{
	int GCStep = (int)(0.01 * lua_gc(LState, LUA_GCCOUNT, 0));

	int ret = lua_gc(LState, LUA_GCSTEP, GCStep);
	if (ret == 1) {
		//_WARN("finished a garbage-collection cycle, current amount of memory %dKB", lua_gc(LState, LUA_GCCOUNT, 0));
	}
}

void signal_exit(int sig, short type, void *arg)
{
	int Ret = CallLua(LState, EXIT_SCRIPT);
	if (0 != Ret)
	{
		_ERROR("call function failed:%d,%s", Ret, EXIT_SCRIPT);
	}
	_ERROR("prepare to exit for %d", sig);
}

void signal_pipe(int sig, short type, void *arg)
{
	_ERROR("engine got sigpipe");
}


int main(int argc, char * *argv)
{
	struct event signal_event[3];
	Logfile = fopen("engine.log", "a");
	if (! Logfile)
	{
		fprintf(stderr, "cann't open engine.log\n");
		return 1;
	}
	setlinebuf(Logfile);
	event_init();
	srand(time(NULL));
	chdir("../script/");
	buckets_t* ud = ff_buckets_t_init(NULL);
	LState = lua_newstate(ff_realloc, ud);
	
	lua_gc(LState, LUA_GCSTOP, 0);
	luaL_openlibs(LState);
	luaopen_lposix(LState);
	luaopen_mongo(LState);
	lua_gc(LState, LUA_GCRESTART, 0);
	luaopen_netlib(LState);
	luaopen_lengine(LState);
	luaopen_msgpack(LState);		//msgpackLua
	init_scene(LState);
	if (luaL_dofile(LState, "./main.lua") != 0 )
	{
		_ERROR("%s", luaL_checkstring(LState, -1));
		return 1;
	}
	
	
	lua_getfield(LState, LUA_GLOBALSINDEX, "GATE_WAY_DATA");
	if (! lua_isnil(LState, -1))
	{
		gateWayData = luaL_checklstring(LState, -1, &gateWayDataLen);
	}
	
	call_multi(gc, NULL, 1);
	event_set(&signal_event[0], SIGINT, EV_SIGNAL|EV_PERSIST, signal_exit, NULL);
	event_add(&signal_event[0], NULL);

	event_set(&signal_event[1], SIGTERM, EV_SIGNAL|EV_PERSIST, signal_exit, NULL);
	event_add(&signal_event[1], NULL);

	event_set(&signal_event[2], SIGPIPE, EV_SIGNAL|EV_PERSIST, signal_pipe, NULL);
	event_add(&signal_event[2], NULL);

	event_dispatch();
	return 0;
}
