#include <msgpack.h>
#include "global.hpp"

#ifndef __MSGPACK_LUA_HPP__
#define __MSGPACK_LUA_HPP__

// msgpackLua_pack: pack lua data
class msgpackLua_pack
{
	public:
		msgpackLua_pack();
		virtual ~msgpackLua_pack();
		
		int packLua(lua_State *L);
		
		void pack(lua_State *L, int index);
		void packNil(lua_State *L, int index);
		void packNumber(lua_State *L, int index);
		void packBoolean(lua_State *L, int index);
		void packString(lua_State *L, int index);
		void packTable(lua_State *L, int index);
		void packTableAsArray(lua_State *L, int index);
		void packTableAsTable(lua_State *L, int index);
		
	protected:
		msgpack_packer* packer;
		msgpack_sbuffer* buffer;
	
	private:
		msgpackLua_pack(const msgpackLua_pack&);
		msgpackLua_pack& operator = (const msgpackLua_pack&);
		
};

// msgpackLua_unpack: unpack lua data
class msgpackLua_unpack
{
	public:
		msgpackLua_unpack();
		virtual ~msgpackLua_unpack();
		
		int unpackLua(lua_State *L);
		
		void unpack(lua_State *L, msgpack_object &msg);
		void unpackArray(lua_State *L, msgpack_object_array &array);
		void unpackTable(lua_State *L, msgpack_object_map &table);
		
	protected:
		msgpack_unpacked unpacked;		
		
	private:
		msgpackLua_unpack(const msgpackLua_unpack&);
		msgpackLua_unpack& operator = (const msgpackLua_unpack&);
};

int packLua(lua_State* L);
int unpackLua(lua_State *L);
void luaopen_msgpack(lua_State* L);

#endif
