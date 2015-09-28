#include "global.hpp"
#include "lstack.hpp"
#include "luacall.hpp"

class TArgNumber:public TArg
{
	double _number;
	public:
		explicit TArgNumber(double v):_number(v){}
		virtual void pushvalue(lua_State* L) const {lua_pushnumber(L, _number);}
		virtual std::string Desc() const {
			std::ostringstream stm;
			stm << "ArgNumber:" << _number << ";";
			return stm.str();
		}
};

class TArgStr:public TArg
{
	std::string _strv;
	public:
		explicit TArgStr(const std::string& v):_strv(v){}
		virtual void pushvalue(lua_State* L) const {lua_pushlstring(L, _strv.c_str(), _strv.length());}
		virtual std::string Desc() const {
			std::ostringstream stm;
			stm << "ArgString:" << _strv << ";";
			return stm.str();
		}
};

class TArgBool:public TArg
{
	bool _boolv;
	public:
		explicit TArgBool(bool v):_boolv(v){}
		virtual void pushvalue(lua_State* L) const {lua_pushboolean(L, _boolv);}
		virtual std::string Desc() const {
			std::ostringstream stm;
			stm << "ArgBool:" << _boolv << ";";
			return stm.str();
		}
};
class TArgNil:public TArg
{
	public:
		explicit TArgNil(){}
		virtual void pushvalue(lua_State* L) const {lua_pushnil(L);}
		virtual std::string Desc() const {
			std::ostringstream stm;
			stm << "ArgNil:nil;";
			return stm.str();
		}
};

void TArgPool::AddArg(double Value)
{
	TArgNumber* pObj = new TArgNumber(Value);
	ArgList.push_back(pObj);
	//std::cout << "add number arg " << Value << std::endl;
}
void TArgPool::AddArg(const std::string& Str)
{
	TArgStr* pObj = new TArgStr(Str);
	ArgList.push_back(pObj);
	//std::cout << "add string arg " << Str << std::endl;
}
void TArgPool::AddArg(bool Value)
{
	TArgBool* pObj = new TArgBool(Value);
	ArgList.push_back(pObj);
	//std::cout << "add bool arg " << Value << std::endl;
}
void TArgPool::AddArg()
{
	TArgNil* pObj = new TArgNil();
	ArgList.push_back(pObj);
	//std::cout << "add nil arg" << std::endl;
}
std::string TArgPool::Desc() const{
	std::string Desc;
	for ( size_t i =0; i < ArgList.size(); i++)
		Desc += ArgList[i]->Desc();
	return Desc;
}
int TArgPool::Push(lua_State* L)const
{
	for (size_t i = 0; i < ArgList.size(); i++)
	{
		ArgList[i]->pushvalue(L);
	}
	return ArgList.size();
}
TArgPool::~TArgPool()
{
	for (size_t i = 0; i < ArgList.size(); i++)
	{
		delete ArgList[i];
	}
	ArgList.clear();
}
int CallLua(lua_State* L, const char* fname, const TArgPool& ArgPoolObj)
{       
	lua_getglobal(L, fname);
	if (lua_isfunction(L, -1) )
	{
		if ( 0 == callscript(L, ArgPoolObj.Push(L), LUA_MULTRET) )
			return 0;
		_ERROR("call function failed:%s", luaL_checkstring(L, -1));
	}
	return -1;
}
int CallLua(lua_State* L, int funref, const TArgPool& ArgPoolObj)
{
	if (funref < 0)
		return -2;
	lua_getref(L, funref);
	if (lua_isfunction(L, -1) )
	{
		if ( 0 == callscript(L, ArgPoolObj.Push(L), LUA_MULTRET) )
			return 0;
		_ERROR("call function failed:%s", luaL_checkstring(L, -1));
	}
	return -1;
}
int CallLua(lua_State* L, const char* fname)
{
	TArgPool ArgPoolObj;
	return CallLua(L, fname, ArgPoolObj);
}
