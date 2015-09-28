#ifndef NEW_AOI
#define NEW_AOI

#include <map>
#include <set>
#include <vector>
#include "lua.hpp"
#include "lauxlib.h"
#include "hash.hpp"

const int GridWidth = 32;
const int GridHeight = 32;

class CScene;
extern hash_table *pSceneTbl;
extern void NewScene(int id, int scene_width, int scene_height);
extern void RemoveScene(int id);

extern void GetNeighbor(lua_State *L, const char * char_id, int id, int x, int y, int aoi_grid);
extern void GetExNeighbor(lua_State *L, const char * char_id, int id, int x, int y);

extern void AddPlayer(int id, const char * char_id, int pos_x, int pos_y, int aoi_grid, void *p_context);
extern void RemovePlayer(int id, const char * char_id, int pos_x, int pos_y, int aoi_grid, void *p_context);

extern void Move(int id, const char * char_id, int src_x, int src_y, int dst_x, int dst_y, int aoi_grid, void *p_context);

struct Point {
	int x;
	int y;
};

struct GridRect {
	int left_index;
	int right_index;
	int top_index;
	int bottom_index;
};

inline bool IsEffectiveRect(const struct GridRect *p_rect);
inline bool InRect(int x, int y, const struct GridRect *p_rect);

class CGrid {
	private:
		std::set<const char *> player_set; // 位于该网格的玩家set
		std::set<const char *> be_seen_set; // 能看到该网格的玩家set

	public:
		CGrid();
		~CGrid();

		std::set<const char *> * GetSeeSet()
		{
			return &player_set;
		}

		std::set<const char *> * GetBeSeenSet()
		{
			return &be_seen_set;
		}

		inline void AddPlayer(const char * char_id);
		inline void AddBeSeen(const char * char_id);

		inline void RemovePlayer(const char * char_id);
		inline void RemoveBeSeen(const char * char_id);
};

class CScene {
	private:
		void **p_grid_set;

		int scene_id;

		int grid_width; // 网格宽
		int grid_height; // 网格高

		int screen_width; // 场景宽
		int screen_height; // 场景高

		int grid_row_count; // 每行有多少个网格
		int grid_column_count; // 每列有多少个网格

		int max_grid_index; // 转换成一维index的最大值

		CGrid * QueryGrid(int grid_index);

		void AddBeSeen(const char * char_id, const std::vector<CGrid *> *p_enter_char_index);
		void RemoveBeSeen(const char * char_id, const std::vector<CGrid *> *p_leave_char_index);

		inline void CalculateAOIRect(struct GridRect *p_ret_rect, int x_index, int y_index, int aoi_grid);
		bool EnterPos(const char * char_id, int enter_index, const std::vector<CGrid *> *p_enter_char_index, const std::vector<const char *> *p_enter_other, void *p_context);
		bool LeavePos(const char * char_id, int leave_index, const std::vector<CGrid *> *p_leave_char_index, const std::vector<const char *> *p_leave_other, void *p_context, int flag);
		inline void CalSubBeSeen(int src_index, int dst_index, std::vector<const char *> *p_sub_result);
		inline void CalSubSee(struct GridRect *p_src_rect, struct GridRect *p_dst_rect, std::vector<CGrid *> *p_sub_result);
		int AddRemoveCommon(const struct Point *p_point, int lua_aoi, std::vector<CGrid *> *p_char_index, std::vector<const char *> *p_other);
		inline bool ChangeGrid(int src_x_index, int src_y_index, int dst_x_index, int dst_y_index);
		inline bool ChangeGridAOI(const struct GridRect *p_src_rect, const struct GridRect *p_dst_rect);
		inline bool CheckPosition(int x, int y);
		inline bool CheckPosition(const struct Point *p_point);

	public:
		CScene(int id, int _screen_width, int _screen_height);
		virtual ~CScene();

		bool AddPlayer(const char * char_id, const struct Point *p_point, int aoi_grid, void *p_context);
		bool RemovePlayer(const char * char_id, const struct Point *p_point, int aoi_grid, void *p_context);

		bool Move(const char * char_id, const struct Point *p_src_point, const struct Point *p_dst_point, int aoi_grid, void *p_context);

		void GetNeighbor(lua_State *L, const char * char_id, int x, int y, int aoi_grid);
		void GetExNeighbor(lua_State *L, const char * char_id, int x, int y);
};
#endif
