#include "msgpackLua.hpp"

msgpackLua_pack *mLuaPacker;
msgpackLua_unpack *mLuaUnpacker;


msgpackLua_pack::msgpackLua_pack()
{
	buffer = msgpack_sbuffer_new();
	msgpack_sbuffer_init(buffer);
	packer = msgpack_packer_new(buffer, msgpack_sbuffer_write);
}

msgpackLua_pack::~msgpackLua_pack()
{
	msgpack_sbuffer_free(buffer);
	msgpack_packer_free(packer);
}

int msgpackLua_pack::packLua(lua_State *L)
{
	pack(L, 1);	
	lua_pushlstring(L, buffer->data, buffer->size);
	lua_pushnumber(L, buffer->size);
	
	msgpack_sbuffer_clear(buffer);
	return 2;
}


void msgpackLua_pack::pack(lua_State *L, int index)
{
	int type = lua_type(L, index);
	switch(type)
	{
		case LUA_TNUMBER:
			packNumber(L, index);
			break;
		case LUA_TBOOLEAN:
			packBoolean(L, index);
			break;
		case LUA_TSTRING:
			packString(L, index);
			break;
		case LUA_TTABLE:
			packTable(L, index);
			break;
		case LUA_TNIL:
			packNil(L, index);
			break;
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		default:
		  luaL_error(L, "invalid type for pack: %s",
					 lua_typename(L, type));
		  break;
    }
}


void msgpackLua_pack::packNil(lua_State *L, int index)
{
	msgpack_pack_nil(packer);
}


void msgpackLua_pack::packNumber(lua_State *L, int index)
{
	double n = lua_tonumber(L, index);
    int64_t i = static_cast<int64_t>(n);
    if (i == n)
		msgpack_pack_int64(packer, i);
    else 
		msgpack_pack_double(packer, n);
}

void msgpackLua_pack::packBoolean(lua_State *L, int index)
{
	int b = lua_toboolean(L, index);
	if (b != 0)
		msgpack_pack_true(packer);
	else
		msgpack_pack_false(packer);
}

void msgpackLua_pack::packString(lua_State *L, int index)
{
	size_t len;
	const char *str = lua_tolstring(L, index, &len);
	if (str == NULL)
	{
		luaL_error(L, "lua_tolstring failed for index %d on type %s.", index, lua_type(L, index));
		return;
	}
	msgpack_pack_raw(packer, len);
	msgpack_pack_raw_body(packer, str, len);
}

void msgpackLua_pack::packTable(lua_State *L, int index)
{
	bool is_array = false;
	size_t len = lua_objlen(L, index);		/*取出table中的array部分长度*/
	if (len > 0)
	{
		lua_pushnumber(L, len);
		if (lua_next(L, index) == 0)		/*以数组长度为key，取出table中的下一对key-value值*/
			is_array = true;				/*如果返回0，则说明table到此为止，因此是纯array*/
		else								/*不然则说明包含hash部分，不是纯array*/
			lua_pop(L, 2);
	}
	if (is_array) 
		packTableAsArray(L, index);
	else
		packTableAsTable(L, index);
}

void msgpackLua_pack::packTableAsTable(lua_State *L, int index)
{
	size_t len = 0;
	lua_pushnil(L);
	while (lua_next(L, index) != 0)
	{
		len++;
		lua_pop(L, 1);
	}
	msgpack_pack_map(packer, len);
	
	int n = lua_gettop(L);
	lua_pushnil(L);
	while(lua_next(L, index) != 0)
	{
		pack(L, n + 1);
		pack(L, n + 2);
		lua_pop(L, 1);
	}
}

void msgpackLua_pack::packTableAsArray(lua_State *L, int index)
{
	int n = lua_gettop(L);
	size_t len = lua_objlen(L, index);
	
	msgpack_pack_array(packer, len);
	for (size_t i = 1; i <= len; i++)
	{
		lua_rawgeti(L, index, i);
		pack(L, n + 1);
		lua_pop(L, 1);
	}
}



msgpackLua_unpack::msgpackLua_unpack()
{
	msgpack_unpacked_init(&unpacked);
}

msgpackLua_unpack::~msgpackLua_unpack()
{
	msgpack_unpacked_destroy(&unpacked);
}


int msgpackLua_unpack::unpackLua(lua_State *L)
{	
	size_t len;
	const char *data = lua_tolstring(L, 1, &len);
	
	if (!msgpack_unpack_next(&unpacked, data, len, NULL))
	{
		luaL_error(L, "unpack failed on index %d and len %d.", 1, len);
		return 0;
	}
	unpack(L, unpacked.data);
	
	return 1;
}

void msgpackLua_unpack::unpack(lua_State *L, msgpack_object &msg)
{
	switch (msg.type) 
	{
		case MSGPACK_OBJECT_NIL:
			lua_pushnil(L);
			break;

		case MSGPACK_OBJECT_BOOLEAN:
			lua_pushboolean(L, msg.via.boolean);
			break;
		case MSGPACK_OBJECT_POSITIVE_INTEGER:
			lua_pushnumber(L, msg.via.u64);
			break;

		case MSGPACK_OBJECT_NEGATIVE_INTEGER:
			lua_pushnumber(L, msg.via.i64);
			break;

		case MSGPACK_OBJECT_DOUBLE:
			lua_pushnumber(L, msg.via.dec);
			break;

		case MSGPACK_OBJECT_RAW:
			lua_pushlstring(L, msg.via.raw.ptr, msg.via.raw.size);
			break;

		case MSGPACK_OBJECT_ARRAY:
			unpackArray(L, msg.via.array);
			break;

		case MSGPACK_OBJECT_MAP:
			unpackTable(L, msg.via.map);
			break;

		default:
			luaL_error(L, "invalid type for unpack: %d", msg.type);
			return;
	}
}


void msgpackLua_unpack::unpackArray(lua_State *L, msgpack_object_array &array)
{
	lua_newtable(L);
	for (uint32_t i = 0; i < array.size; i++) {
		unpack(L, array.ptr[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void msgpackLua_unpack::unpackTable(lua_State *L, msgpack_object_map &table)
{
	lua_newtable(L);
	for (uint32_t i = 0; i < table.size; i++) {
		unpack(L, table.ptr[i].key);
		unpack(L, table.ptr[i].val);
		lua_rawset(L, -3);
	}
}

int packLua(lua_State* L)
{
	return mLuaPacker->packLua(L);
}

int unpackLua(lua_State *L)
{
	return mLuaUnpacker->unpackLua(L);
}

void luaopen_msgpack(lua_State* L)
{
	mLuaPacker = new msgpackLua_pack;
	mLuaUnpacker = new msgpackLua_unpack;
	
	const struct luaL_reg msgpack[] = {
		{"pack", packLua},
		{"unpack", unpackLua},
		{NULL, NULL}
	};
	luaL_register(L, "msgpack", msgpack);
}
