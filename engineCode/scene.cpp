#include "scene.hpp"
#include "lstack.hpp"
#include <math.h>

hash_table *pSceneTbl = NULL;

void GetNeighbor(lua_State *L, const char * char_id, int id, int x, int y, int lua_aoi)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	pscene -> GetNeighbor(L, char_id, x, y, lua_aoi);
}

void GetExNeighbor(lua_State *L, const char * char_id, int id, int x, int y)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	pscene -> GetExNeighbor(L, char_id, x, y);
}

void NewScene(int id, int screen_width, int screen_height)
{
	void * p;
	if (find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	pscene = new CScene(id, screen_width, screen_height);
	insert_inode(pSceneTbl, id, (void *)pscene);
}

void RemoveScene(int id)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	delete pscene;
	delete_inode(pSceneTbl, id);
}

void AddPlayer(int id, const char * char_id, int pos_x, int pos_y, int lua_aoi, void *p_context)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	struct Point temp_point;
	temp_point.x = pos_x;
	temp_point.y = pos_y;
	pscene -> AddPlayer(char_id, &temp_point, lua_aoi, p_context);
}

void RemovePlayer(int id, const char * char_id, int pos_x, int pos_y, int lua_aoi, void *p_context)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;
	struct Point temp_point;
	temp_point.x = pos_x;
	temp_point.y = pos_y;
	pscene -> RemovePlayer(char_id, &temp_point, lua_aoi, p_context);
}

void Move(int id, const char * char_id, int src_x, int src_y, int dst_x, int dst_y, int lua_aoi, void *p_context)
{
	void * p;
	if (!find_inode(pSceneTbl, id, &p))
	{
		return;
	}

	CScene *pscene = (CScene*)p;

	struct Point src_point;
	src_point.x = src_x;
	src_point.y = src_y;

	struct Point dst_point;
	dst_point.x = dst_x;
	dst_point.y = dst_y;

	pscene -> Move(char_id, &src_point, &dst_point, lua_aoi, p_context);
}

// ********************************************** CGrid
CGrid::CGrid()
{
}

CGrid::~CGrid()
{
}

inline void CGrid::AddPlayer(const char * char_id)
{
	player_set.insert(char_id);
}

inline void CGrid::AddBeSeen(const char * char_id)
{
	be_seen_set.insert(char_id);
}

inline void CGrid::RemoveBeSeen(const char * char_id)
{
	be_seen_set.erase(char_id);
}

inline void CGrid::RemovePlayer(const char * char_id)
{
	player_set.erase(char_id);
}
// ************************************************ CScene
CScene::CScene(int id, int _screen_width, int _screen_height):scene_id(id), screen_width(_screen_width), screen_height(_screen_height)
{
	grid_width = GridWidth;
	grid_height = GridHeight;

	grid_row_count = ceil((float)(screen_width) / grid_width);
	grid_column_count = ceil((float)(screen_height) / grid_height);

	max_grid_index = grid_row_count * grid_column_count - 1;

	p_grid_set = new void *[max_grid_index + 1];

	for (int i = 0; i <= max_grid_index; i++)
	{
		p_grid_set[i] = NULL;
	}
}

CScene::~CScene()
{
	for (int i = 0; i <= max_grid_index; i++)
	{
		delete ((CGrid *)(p_grid_set[i]));
	}

	delete []p_grid_set;
}

void CScene::GetNeighbor(lua_State *L, const char * char_id, int x, int y, int lua_aoi)
{
	if (!CheckPosition(x, y))
	{
		return;
	}

	struct GridRect grid_rect;
	CalculateAOIRect(&grid_rect, x, y, lua_aoi);

	int key = 1;
	CGrid *p_temp_grid = NULL;
	std::set<const char *> *p_see_set = NULL;
	std::set<const char *>::iterator temp_it;
	for (int temp_i = grid_rect.top_index; temp_i <= grid_rect.bottom_index; temp_i ++)
	{
		for (int temp_j = grid_rect.left_index; temp_j <= grid_rect.right_index; temp_j ++)
		{
			p_temp_grid = QueryGrid(temp_i * grid_row_count + temp_j);
			if (!p_temp_grid)
			{
				return;
			}
			p_see_set = p_temp_grid -> GetSeeSet();
			for (temp_it = p_see_set -> begin(); temp_it != p_see_set -> end(); ++ temp_it)
			{
				if (strcmp(*temp_it, char_id) == 0) continue;

				lua_pushnumber(L, key);
				lua_pushstring(L, (*temp_it));
				lua_settable(L, -3);

				++ key;
			}
		}
	}
}

void CScene::GetExNeighbor(lua_State *L, const char * char_id, int x, int y)
{
	if (!CheckPosition(x, y))
	{
		return;
	}

	int grid_index = ((y / grid_height) * grid_row_count + (x / grid_width));
	CGrid *p_temp_grid = QueryGrid(grid_index);
	if (!p_temp_grid)
	{
		return;
	}
	std::set<const char *> *p_be_seen_set = p_temp_grid -> GetBeSeenSet();
	std::set<const char *>::iterator temp_it;
	int key = 1;
	for (temp_it = p_be_seen_set -> begin(); temp_it != p_be_seen_set -> end(); ++ temp_it)
	{
		if (strcmp((*temp_it), char_id) == 0) continue;

		lua_pushnumber(L, key);
		lua_pushstring(L, (*temp_it));
		lua_settable(L, -3);

		++ key;
	}
}

CGrid * CScene::QueryGrid(int grid_index)
{
	CGrid *p_grid = NULL;

	if ((grid_index > max_grid_index) || (grid_index < 0))
	{
		_RUNTIME_ERROR("%d query wrong grid_index %d\n", scene_id, grid_index);
		return NULL;
	}

	if (p_grid_set[grid_index] == NULL)
	{
		p_grid = new CGrid();
		p_grid_set[grid_index] = (void *)p_grid;
	}

	return (CGrid *)(p_grid_set[grid_index]);
}

inline void CScene::CalculateAOIRect(struct GridRect *p_ret_rect, int x, int y, int lua_aoi)
{
/*	int left = x - lua_aoi + 1;
	if (left < 0)
	{
		left = 0;
	}

	int right = x + lua_aoi - 1;
	if (right > (screen_width - 1))
	{
		right = screen_width - 1;
	}

	int top = y - lua_aoi + 1;
	if (top < 0)
	{
		top = 0;
	}

	int bottom = y + lua_aoi -1;
	if (bottom > (screen_height - 1))
	{
		bottom = screen_height - 1;
	}*/
	int left = x - lua_aoi + 1;
	int right = x + lua_aoi - 1;
	if (x < lua_aoi)
	{
		left = 0;
		right = lua_aoi * 2;
		if (right > screen_width - 1)
			right = screen_width - 1;
	}
	else if (x > (screen_width - 1 - lua_aoi))
	{
		right = screen_width - 1;
		left = screen_width - 1 - lua_aoi * 2;
		if (left < 0)
			left = 0;
	}
	
	int top = y - lua_aoi + 1;
	int bottom = y + lua_aoi -1;
	if (y < lua_aoi)
	{
		top = 0;
		bottom = lua_aoi * 2;
		if(bottom > screen_height - 1)
			bottom = screen_height - 1;
	}
	else if (y > screen_height - 1 - lua_aoi)
	{
		bottom = screen_height - 1;
		top = screen_height - 1 - lua_aoi * 2;
		if (top < 0)
			top = 0;
	}

	p_ret_rect -> left_index = left / grid_width;
	p_ret_rect -> right_index = right / grid_width;
	p_ret_rect -> top_index = top / grid_height;
	p_ret_rect -> bottom_index = bottom / grid_height;
}

void CScene::AddBeSeen(const char * char_id, const std::vector<CGrid *> *p_enter_grid)
{
	std::vector<CGrid *>::const_iterator it;
	for (it = p_enter_grid -> begin(); it != p_enter_grid -> end(); ++it)
	{
		if (!(*it))
		{
			_RUNTIME_ERROR("%d %s add beseen null pointer;\n", scene_id, char_id);
			return;
		}
		(*it) -> AddBeSeen(char_id);
	}
}

void CScene::RemoveBeSeen(const char * char_id, const std::vector<CGrid *> *p_leave_grid)
{
	std::vector<CGrid *>::const_iterator it;
	for (it = p_leave_grid -> begin(); it != p_leave_grid -> end(); ++it)
	{
		if (!(*it))
		{
			_RUNTIME_ERROR("%d %s remove beseen null pointer;\n", scene_id, char_id);
			return;
		}
		(*it) -> RemoveBeSeen(char_id);
	}
}

int CScene::AddRemoveCommon(const struct Point *p_point, int lua_aoi, std::vector<CGrid *> *p_grid, std::vector<const char *> *p_other)
{
	if (lua_aoi > 0)
	{
		struct GridRect grid_aoi_rect;
		CalculateAOIRect(&grid_aoi_rect, p_point -> x, p_point -> y, lua_aoi);
		CGrid *p_temp_grid = NULL;

		for (int i = grid_aoi_rect.top_index; i <= grid_aoi_rect.bottom_index; i++)
		{
			for (int j = grid_aoi_rect.left_index; j <= grid_aoi_rect.right_index; j++)
			{
				p_temp_grid = QueryGrid(i * grid_row_count + j);
				if (!p_temp_grid)
				{
					_RUNTIME_ERROR("%d add_remove wrong grid %d, %d", scene_id, p_point -> x, p_point -> y);
					return -1;
				}

				p_grid -> push_back(p_temp_grid);
			}
		}
	}

	int x_index = (p_point -> x) / grid_width;
	int y_index = (p_point -> y) / grid_height;

	int grid_index = y_index * grid_row_count + x_index;

	CGrid *p_temp_grid = QueryGrid(grid_index);
	if (!p_temp_grid)
	{
		_RUNTIME_ERROR("%d add_remove in wrong grid %d", scene_id, grid_index);
		return -1;
	}

	std::set<const char *> *p_be_seen_set = p_temp_grid -> GetBeSeenSet();

	std::set<const char *>::iterator temp_it;
	for (temp_it = p_be_seen_set -> begin(); temp_it != p_be_seen_set -> end(); ++ temp_it)
	{
		p_other -> push_back(*temp_it);
	}

	return grid_index;
}

bool CScene::AddPlayer(const char * char_id, const struct Point *p_point, int lua_aoi, void *p_context)
{
	if (!CheckPosition(p_point))
	{
		_RUNTIME_ERROR("%d add player %s error %d, %d", scene_id, char_id, p_point -> x, p_point -> y);
		return false;
	}

	std::vector<CGrid *> enter_grid;
	std::vector<const char *> enter_other;
	int enter_index = AddRemoveCommon(p_point, lua_aoi, &enter_grid, &enter_other);
	if (enter_index < 0)
	{
		_RUNTIME_ERROR("%d add player %s error index %d", scene_id, char_id, enter_index);
		return false;
	}

	return EnterPos(char_id, enter_index, &enter_grid, &enter_other, p_context);
}

bool CScene::RemovePlayer(const char * char_id, const struct Point *p_point, int lua_aoi, void *p_context)
{
	if (!CheckPosition(p_point))
	{
		_RUNTIME_ERROR("%d remove player %s error %d, %d", scene_id, char_id, p_point -> x, p_point -> y);
		return false;
	}

	std::vector<CGrid *> leave_grid;
	std::vector<const char *> leave_other;
	int leave_index = AddRemoveCommon(p_point, lua_aoi, &leave_grid, &leave_other);
	if (leave_index < 0)
	{
		_RUNTIME_ERROR("%d remove player %s error index %d", scene_id, char_id, leave_index);
		return false;
	}

	return LeavePos(char_id, leave_index, &leave_grid, &leave_other, p_context, 1);
}

static void LeftAOIProc(const char * char_id, const std::vector<CGrid *> *p_grid, lua_State *L, int flag)
{
	lua_pushstring(L, char_id);

	int key = 1;
	std::vector<CGrid *>::const_iterator it;
	std::set<const char *> *p_see_set = NULL;
	std::set<const char *>::iterator temp_it;

	lua_newtable(L);

	for (it = p_grid -> begin(); it != p_grid -> end(); ++ it)
	{
		if (!(*it))
		{
			return;
		}
		p_see_set = (*it) -> GetSeeSet();
		for (temp_it = p_see_set -> begin(); temp_it != p_see_set -> end(); ++ temp_it)
		{
			lua_pushnumber(L, key);
			lua_pushstring(L, (*temp_it));
			lua_settable(L, -3);
			++ key;
		}
	}

	lua_pushnumber(L, flag);
	
	if (lua_isfunction(L, -4))
	{
		if (callscript(L, 3, LUA_MULTRET) != 0)
		{
			// 错误处理 ???
		}
	}
}

static void OnEnterLeftAOI(const char * enter_char_id, const std::vector<CGrid *> *p_enter_grid, void *p_context)
{
	lua_State *L = (lua_State*)p_context;
	lua_getglobal(L, "CallFromEngineOnEnterLeft");
	LeftAOIProc(enter_char_id, p_enter_grid, L, 0);
}

static void OnLeaveLeftAOI(const char * leave_char_id, const std::vector<CGrid *> *p_leave_grid, void *p_context, int flag)
{
	lua_State *L = (lua_State*)p_context;
	lua_getglobal(L, "CallFromEngineOnLeaveLeft");
	LeftAOIProc(leave_char_id, p_leave_grid, L, flag);
}

static void RightAOIProc(const char * char_id, const std::vector<const char *> *p_other, lua_State *L)
{
	lua_pushstring(L, char_id);

	lua_newtable(L);
	int key = 1;

	std::vector<const char *>::const_iterator it;
	for (it = (p_other -> begin()); it != (p_other -> end()); ++ it)
	{
		lua_pushnumber(L, key);
		lua_pushstring(L, (*it));
		lua_settable(L, -3);
		++ key;
	}

	if (lua_isfunction(L, -3))
	{
		if (callscript(L, 2, LUA_MULTRET) != 0)
		{
			// 错误处理 ???
		}
	}
}

static void OnEnterRightAOI(const char * enter_char_id, const std::vector<const char *> *p_enter_other, void *p_context)
{
	lua_State *L = (lua_State*)p_context;
	lua_getglobal(L, "CallFromEngineOnEnterRight");
	RightAOIProc(enter_char_id, p_enter_other, L);
}

static void OnLeaveRightAOI(const char * leave_char_id, const std::vector<const char *> *p_leave_other, void *p_context)
{
	lua_State *L = (lua_State*)p_context;
	lua_getglobal(L, "CallFromEngineOnLeaveRight");
	RightAOIProc(leave_char_id, p_leave_other, L);
}

bool CScene::EnterPos(const char * char_id, int enter_index, const std::vector<CGrid *> *p_enter_grid, const std::vector<const char *> *p_enter_other, void *p_context)
{
	// 维护数据
	if (enter_index > -1)
	{
		CGrid * p_temp_grid = QueryGrid(enter_index);
		if (!p_temp_grid)
		{
			_RUNTIME_ERROR("%d %s enter pos error %d\n", scene_id, char_id, enter_index);
			return false;
		}

		p_temp_grid -> AddPlayer(char_id);
	}

	int t_size = 0;
	if (p_enter_grid)
	{
		t_size = (int)(p_enter_grid -> size());
		if (t_size > 0)
		{
			AddBeSeen(char_id, p_enter_grid);
			// 进入通知
			OnEnterLeftAOI(char_id, p_enter_grid, p_context);
		}
	}

	if (p_enter_other)
	{
		t_size = (int)(p_enter_other -> size());
		if (t_size > 0)
		{
			OnEnterRightAOI(char_id, p_enter_other, p_context);
		}
	}

	return true;
}

bool CScene::LeavePos(const char * char_id, int leave_index, const std::vector<CGrid *> *p_leave_grid, const std::vector<const char *> *p_leave_other, void *p_context, int flag)
{
	// 维护数据
	if (leave_index > -1)
	{
		CGrid * p_temp_grid = QueryGrid(leave_index);
		if (!p_temp_grid)
		{
			_RUNTIME_ERROR("%d %s leave pos error %d\n", scene_id, char_id, leave_index);
			return false;
		}

		p_temp_grid -> RemovePlayer(char_id);
	}

	int t_size = 0;
	if (p_leave_grid)
	{
		t_size = (int)(p_leave_grid -> size());
		if (t_size > 0)
		{
			RemoveBeSeen(char_id, p_leave_grid);
			// 离开通知
			OnLeaveLeftAOI(char_id, p_leave_grid, p_context, flag);
		}
	}

	if (p_leave_other)
	{
		t_size = (int)(p_leave_other -> size());
		if (t_size > 0)
		{
			OnLeaveRightAOI(char_id, p_leave_other, p_context);
		}
	}

	return true;
}

inline void CScene::CalSubBeSeen(int src_index, int dst_index, std::vector<const char *> *p_sub_result)
{
	CGrid *p_src_grid = QueryGrid(src_index);
	CGrid *p_dst_grid = QueryGrid(dst_index);
	if (!p_src_grid || !p_dst_grid)
	{
		return;
	}

	std::set<const char *> *p_src_beseen = p_src_grid -> GetBeSeenSet();
	std::set<const char *>::iterator temp_src_it;
	std::set<const char *> *p_dst_beseen = p_dst_grid -> GetBeSeenSet();

	for (temp_src_it = p_src_beseen -> begin(); temp_src_it != p_src_beseen -> end(); ++ temp_src_it)
	{
		if (p_dst_beseen -> find((*temp_src_it)) == p_dst_beseen -> end())
		{
			p_sub_result -> push_back(*temp_src_it);
		}
	}
}

inline void CScene::CalSubSee(struct GridRect *p_src_rect, struct GridRect *p_dst_rect, std::vector<CGrid *> *p_sub_result)
{
	int grid_index = 0;
	CGrid *p_temp_grid = NULL;
//	std::set<int>::iterator temp_it;

	for (int i = p_src_rect -> top_index; i <= p_src_rect -> bottom_index; i++)
	{
		for (int j = p_src_rect -> left_index; j <= p_src_rect -> right_index; j++)
		{
			if (!InRect(j, i, p_dst_rect))
			{
				grid_index = i * grid_row_count + j;
				p_temp_grid = QueryGrid(grid_index);
				if (p_temp_grid)
				{
					p_sub_result -> push_back(p_temp_grid);
				}
			}
		}
	}
}

inline bool IsEffectiveRect(const struct GridRect *p_rect)
{
	if (p_rect -> left_index == -1 || p_rect -> right_index == -1 || p_rect -> top_index == -1 || p_rect -> bottom_index == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

inline bool InRect(int x, int y, const struct GridRect *p_rect)
{
	if ((x >= p_rect -> left_index) && (x <= p_rect -> right_index) && (y >= p_rect -> top_index) && (y <= p_rect -> bottom_index))
	{
		return true;
	}
	else
	{
		return false;
	}
}

inline bool CScene::ChangeGridAOI(const struct GridRect *p_src_rect, const struct GridRect *p_dst_rect)
{
	if ((p_src_rect -> left_index == p_dst_rect -> left_index) && (p_src_rect -> right_index == p_dst_rect -> right_index) &&
			(p_src_rect -> top_index == p_dst_rect -> top_index) && (p_src_rect -> bottom_index == p_dst_rect -> bottom_index))
	{
		return false;
	}
	return true;
}

inline bool CScene::ChangeGrid(int src_x_index, int src_y_index, int dst_x_index, int dst_y_index)
{
	if ((src_x_index == dst_x_index) && (src_y_index == dst_y_index))
	{
		return false;
	}
	return true;
}

inline bool CScene::CheckPosition(const struct Point *p_point)
{
	if ((p_point -> x < 0) || (p_point -> x > (screen_width - 1)) || (p_point -> y < 0) || (p_point -> y > (screen_height - 1)))
	{
		return false;
	}
	return true;
}

inline bool CScene::CheckPosition(int x, int y)
{
	if ((x < 0) || (x > (screen_width - 1)) || (y < 0) || (y > (screen_height - 1)))
	{
		return false;
	}
	return true;

}

bool CScene::Move(const char * char_id, const struct Point *p_src_point, const struct Point *p_dst_point, int lua_aoi, void *p_context)
{
	bool right_src = CheckPosition(p_src_point);
	bool right_dst = CheckPosition(p_dst_point);

	if (!right_src && !right_dst) // 两个坐标对都不合法
	{
		_RUNTIME_ERROR("%d %s move wrong point src:%d, %d; dst:%d, %d;\n", scene_id, char_id, p_src_point -> x, p_src_point -> y, p_dst_point -> x, p_dst_point -> y);
		return false;
	}
	else if(!right_src) // 当作add player
	{
		_RUNTIME_ERROR("%d %s move wrong point src:%d, %d;\n", scene_id, char_id, p_src_point -> x, p_src_point -> y);
		return AddPlayer(char_id, p_dst_point, lua_aoi, p_context);
	}
	else if(!right_dst) // 当作remove player
	{
		_RUNTIME_ERROR("%d %s move wrong point dst:%d, %d;\n", scene_id, char_id, p_dst_point -> x, p_dst_point -> y);
		return RemovePlayer(char_id, p_src_point, lua_aoi, p_context);
	}

	struct GridRect src_aoi_rect;
	struct GridRect dst_aoi_rect;
	bool change_grid = false;
	bool change_grid_aoi = false;

	std::vector<CGrid *> leave_grid;
	std::vector<CGrid *> enter_grid;

	std::vector<const char *> leave_other; // 离开other的other id
	std::vector<const char *> enter_other; // 进入other的other id

	int leave_index = -1; // -1 代表没有关于grid index的改变
	int enter_index = -1;

	if (lua_aoi > 0)
	{
		CalculateAOIRect(&src_aoi_rect, p_src_point -> x, p_src_point -> y, lua_aoi);
		CalculateAOIRect(&dst_aoi_rect, p_dst_point -> x, p_dst_point -> y, lua_aoi);
		change_grid_aoi = ChangeGridAOI(&src_aoi_rect, &dst_aoi_rect);
		if (change_grid_aoi)
		{
			CalSubSee(&src_aoi_rect, &dst_aoi_rect, &leave_grid);
			CalSubSee(&dst_aoi_rect, &src_aoi_rect, &enter_grid);
		}
	}

	int src_x_index = (p_src_point -> x) / grid_width;
	int src_y_index = (p_src_point -> y) / grid_height;
	int dst_x_index = (p_dst_point -> x) / grid_width;
	int dst_y_index = (p_dst_point -> y) / grid_height;

	change_grid = ChangeGrid(src_x_index, src_y_index, dst_x_index, dst_y_index);
	if (change_grid)
	{
		leave_index = src_y_index * grid_row_count + src_x_index;
		enter_index = dst_y_index * grid_row_count + dst_x_index;
		CalSubBeSeen(leave_index, enter_index, &leave_other);
		CalSubBeSeen(enter_index, leave_index, &enter_other);
	}

	if (!change_grid && !change_grid_aoi)
	{ // 没有更换格子 也没有改变格子视野
		return true;
	}

	if (!LeavePos(char_id, leave_index, &leave_grid, &leave_other, p_context, 0))
	{
		_RUNTIME_ERROR("%d %s leave pos error return src:%d, %d; dst:%d, %d;\n", scene_id, char_id, p_src_point -> x, p_src_point -> y, p_dst_point -> x, p_dst_point -> y);
		return false;
	}

	if (!EnterPos(char_id, enter_index, &enter_grid, &enter_other, p_context))
	{
		_RUNTIME_ERROR("%d %s enter pos error return src:%d, %d; dst:%d, %d;\n", scene_id, char_id, p_src_point -> x, p_src_point -> y, p_dst_point -> x, p_dst_point -> y);
		return false;
	}

	return true;
}
