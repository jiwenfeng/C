#include "lscene.hpp"
#include <cassert>
#include <math.h>

static int get_neighbor(lua_State *L)
{
	const char * char_id = luaL_checkstring(L, 1);
	int SceneId = luaL_checknumber(L, 2);
	int x = luaL_checknumber(L, 3);
	int y = luaL_checknumber(L, 4);

	int lua_aoi = luaL_checknumber(L, 5);

	lua_newtable(L);
	GetNeighbor(L, char_id, SceneId, x, y, lua_aoi);
	return 1;
}

static int get_ex_neighbor(lua_State *L)
{
	const char * char_id = luaL_checkstring(L, 1);
	int SceneId = luaL_checknumber(L, 2);
	int x = luaL_checknumber(L, 3);
	int y = luaL_checknumber(L, 4);

	lua_newtable(L);
	GetExNeighbor(L, char_id, SceneId, x, y);
	return 1;
}

static int new_scene(lua_State *L) // 为脚本层场景对象创建一个场景管理对象
{
	int SceneId = luaL_checknumber(L, 1);
	int SceneWidth = luaL_checknumber(L, 2);
	int SceneHeight = luaL_checknumber(L, 3);

	NewScene(SceneId, SceneWidth, SceneHeight);

	return 0;
}

static int delete_scene(lua_State *L) // 为脚本层场景对象删除一个场景管理对象
{
	int SceneId = luaL_checknumber(L, 1);

	RemoveScene(SceneId);

	return 0;
}

static int add_object(lua_State *L)
{
	int scene_id = luaL_checknumber(L, 1);
	const char * char_id = luaL_checkstring(L, 2);
	int pos_x = luaL_checknumber(L, 3);
	int pos_y = luaL_checknumber(L, 4);

	int lua_aoi = luaL_checknumber(L, 5);

	AddPlayer(scene_id, char_id, pos_x, pos_y, lua_aoi, L);

	return 0;
}

static int remove_object(lua_State *L)
{
	int scene_id = luaL_checknumber(L, 1);
	const char * char_id = luaL_checkstring(L, 2);
	int pos_x = luaL_checknumber(L, 3);
	int pos_y = luaL_checknumber(L, 4);

	int lua_aoi = luaL_checknumber(L, 5);

	RemovePlayer(scene_id, char_id, pos_x, pos_y, lua_aoi, L);

	return 0;
}

static int move(lua_State *L)
{
	int scene_id = luaL_checknumber(L, 1);
	const char * char_id = luaL_checkstring(L, 2);
	int src_x = luaL_checknumber(L, 3);
	int src_y = luaL_checknumber(L, 4);
	int dst_x = luaL_checknumber(L, 5);
	int dst_y = luaL_checknumber(L, 6);

	int lua_aoi = luaL_checknumber(L, 7);

	Move(scene_id, char_id, src_x, src_y, dst_x, dst_y, lua_aoi, L);

	return 0;
}

static const struct luaL_Reg scenelib[] = {
	{"new_scene", new_scene},
	{"delete_scene", delete_scene},
	{"add_object", add_object},
	{"remove_object", remove_object},
	{"get_neighbor", get_neighbor},
	{"get_ex_neighbor", get_ex_neighbor},
	{"move", move},

	{NULL, NULL},
};

int init_scene(lua_State *L)
{
	pSceneTbl = init_hash_table(512);

	luaL_register(L, "lengine.scene", scenelib);

	lua_pushnil(L);
	return 1;
}
