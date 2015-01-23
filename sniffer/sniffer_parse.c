#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <assert.h>
#include "sniffer.h"
#include "sniffer_log.h"
#include "sniffer_buffer.h"

static int _sniffer_parse_push(lua_State *L);
static int _traceback(lua_State *L);
static int _open_libs(lua_State *L);

static const luaL_Reg parse_lib[] = {
	{"push", _sniffer_parse_push},
	{NULL, NULL}
};

static int
_open_libs(lua_State *L)
{
	luaL_newlib(L, parse_lib);
	//luaL_register(L, "parse_lib", parse_lib);
	return 1;
}

static int
_sniffer_parse_push(lua_State *L)
{
	struct sniffer_base *base = (struct sniffer_base *)lua_touserdata(L, 1);
	size_t ss;
	const char *str = lua_tolstring(L, -1, &ss);
	sniffer_buffer_push(base->sb_output, str, ss);
	return 0;
}

static int
_traceback(lua_State *L)
{
	lua_getfield(L, LUA_RIDX_GLOBALS, "debug");
	//lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if(!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);
	lua_pcall(L, 1, 1, 0);
	return 1;
}

int
sniffer_parse_init(struct sniffer_base *base)
{
	//lua_State *L = lua_open();
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	if(0 != luaL_dofile(L, "./script/main.lua"))
	{
		log(base, "%s", lua_tostring(L, -1));
		return -1;
	}
	luaL_requiref(L, "parse", _open_libs, 0);
//	_open_libs(L);
	base->parse_handler = L;
	return 0;
}

int
sniffer_parse_data(struct sniffer_base *base, const char *data, size_t ss)
{
	assert(base->parse_handler);
	lua_State *L = (lua_State *)base->parse_handler;
	lua_pushcfunction(L, _traceback);
	int b = lua_gettop(L);
	lua_getglobal(L, "main");
	if(!lua_isfunction(L, -1))
	{
		report("%s", "undefined reference to `main'");
		return -1;
	}
	lua_pushlightuserdata(L, base);
	lua_pushlstring(L, data, ss);
	lua_pushnumber(L, ss);
	int result = lua_pcall(L, 3, 1, b);
	if(0 != result)
	{
		log(base, "%s", lua_tostring(L, -1));
		return -1;
	}
	result = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);
	return result == -1 ? -1 : result;
}

void
sniffer_parse_cleanup(void *handler)
{
	if(!handler)
	{
		return;
	}
	lua_State *L = (lua_State *)handler;
	lua_close(L);
}
