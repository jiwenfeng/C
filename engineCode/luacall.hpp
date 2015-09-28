#ifndef __LUA_CALL_HPP__
#define __LUA_CALL_HPP__
#include<vector>
#include<string>
class TArg
{
	public:
		TArg(){}
		virtual void pushvalue(lua_State* L)const = 0;
		virtual std::string Desc()const = 0;
		virtual ~TArg(){}
};
class TArgPool
{
	std::vector<TArg*> ArgList;
	public:
		TArgPool(){}
		void AddArg(double Value);
		void AddArg(const std::string& Str);
		void AddArg(bool Value);
		void AddArg();
		int Push(lua_State* L)const;
		size_t Size() const { return ArgList.size();}
		std::string Desc() const;
		~TArgPool();
};
int CallLua(lua_State* L, const char* fname, const TArgPool& ArgPoolObj);
int CallLua(lua_State* L, int funref, const TArgPool& ArgPoolObj);
int CallLua(lua_State* L, const char* fname);

#endif
