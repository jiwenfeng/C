//$Id
#include <string>
#include <fstream>
#include <sstream>

#include "lstack.hpp"
#include "api.hpp"

static void InfiniteLoop(lua_State *L, lua_Debug *de)
{
	lua_getglobal (L, "debug");
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
		return;
	lua_pushstring(L, "infinite loop");
	callscript(L, 1, LUA_MULTRET);
	const char* traceback = luaL_checkstring(L, -1);
	_ERROR("%s", traceback);
	//CallLua(L, EXIT_SCRIPT);
}

static int traceback (lua_State *L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  
	lua_pcall(L, 1, 1, 0);  //call debug.traceback
	return 1;
}

int callscript(lua_State *L, int narg, int nresults)
{
	int status;
	lua_sethook(L, InfiniteLoop, LUA_MASKCOUNT, 250000000);
	int base = lua_gettop(L) - narg;  //function index
	lua_pushcfunction(L, traceback);  //push traceback function
	lua_insert(L, base);  //put it under chunk and args
	status = lua_pcall(L, narg, nresults, base);
	lua_remove(L, base);  //remove traceback function
	return status;
}

