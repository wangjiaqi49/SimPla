/*Copyright (C) 2007-2011 YU Zhi. All rights reserved.
 * $Id: LuaWrap.h 1005 2011-01-27 09:15:38Z salmon $
 * luaWrap.h
 *
 *  Created on: 2010-9-22
 *      Author: salmon
 */

#ifndef INCLUDE_LUA_PARSER_H_
#define INCLUDE_LUA_PARSER_H_

#include <lua5.2/lua.hpp>
#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <stdlib.h>
#include  <map>
#include "../fetl/ntuple.h"
#include "log.h"
#include "utilities.h"

namespace simpla
{

#define LUA_ERROR(_L, _MSG_)  ERROR<< (_MSG_)<<std::string("\n") << lua_tostring(_L, 1) ;   lua_pop(_L, 1);

static void stackDump(lua_State *L)
{
	int top = lua_gettop(L);
	for (int i = 1; i < top; ++i)
	{
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING:
			std::cout << "[" << i << "]" << lua_tostring(L,i) << std::endl;
			break;

		case LUA_TBOOLEAN:
			std::cout << "[" << i << "]" << std::boolalpha
					<< lua_toboolean(L, i) << std::endl;
			break;

		case LUA_TNUMBER:
			std::cout << "[" << i << "]" << lua_tonumber(L, i) << std::endl;
			break;
		case LUA_TTABLE:
			std::cout << "[" << i << "]" << "is a table" << std::endl;
			break;
		default:
			std::cout << "[" << i << "]" << "is an unknown type" << std::endl;
		}
	}
	std::cout << "===== End the listing =====" << std::endl;
}

class LuaIterator;
class LuaObject;

template<typename T> struct LuaTrans;

inline void ToLua(lua_State*L)
{
}
template<typename T, typename ... Args>
inline void ToLua(lua_State*L, T const & v, Args const & ... rest)
{
	LuaTrans<T>::To(L, v);
	ToLua(L, rest...);
}

inline void FromLua(lua_State*L, int)
{
}

template<typename T, typename ... Args>
inline void FromLua(lua_State*L, int idx, T & v, Args & ... rest)
{
	LuaTrans<T>::From(L, idx, v);
	FromLua(L, idx + 1, rest...);
}

struct LuaState
{
	std::shared_ptr<LuaState> parent_;
	std::shared_ptr<lua_State> lstate_;
	int idx_;
	std::string path_;
	bool is_weak_;
	LuaState() :
			lstate_(luaL_newstate(), lua_close), idx_(LUA_RIDX_GLOBALS), path_(
					"[root]"), is_weak_(false)
	{
		luaL_openlibs(lstate_.get());
	}

	LuaState(LuaState const &r) :
			parent_(r.parent_), lstate_(r.lstate_), idx_(r.idx_), path_(
					r.path_), is_weak_(r.is_weak_)
	{

	}
	LuaState(std::shared_ptr<LuaState> p, int idx, std::string path,
			bool is_weak = false) :
			parent_(p), lstate_(p->lstate_), idx_(idx), path_(
					p->path_ + "." + path), is_weak_(is_weak)
	{
	}

	LuaState(std::shared_ptr<lua_State> l, int idx, bool is_weak = false) :
			lstate_(l), idx_(idx), path_("<GLOBAL>.[" + ToString(idx) + "]"), is_weak_(
					is_weak)
	{
	}
	~LuaState()
	{
		if (!is_weak_ && idx_ != 0 && idx_ != LUA_RIDX_GLOBALS)
		{
			lua_remove(lstate_.get(), idx_);
		}

	}
};

class LuaObject
{
	std::shared_ptr<LuaState> self_;

	lua_State* lstate_;

	void Init()
	{
		if (self_ == nullptr)
		{
			self_ = std::shared_ptr<LuaState>(new LuaState);
			lstate_ = self_->lstate_.get();
		}

	}
public:

	LuaObject() :
			self_(nullptr), lstate_(nullptr)

	{
	}
	LuaObject(LuaObject const & r) :
			self_(r.self_), lstate_(r.lstate_)
	{

	}

	LuaObject(std::shared_ptr<LuaState> lstate) :
			self_(lstate), lstate_(self_->lstate_.get())
	{

	}
	LuaObject(LuaState* lstate) :
			self_(lstate), lstate_(self_->lstate_.get())
	{

	}

	~LuaObject()
	{
	}

	lua_State * GetState()
	{
		return self_->lstate_.get();
	}
	inline void ParseFile(std::string const & filename)
	{
		Init();
		if (filename != "" && luaL_dofile(lstate_, filename.c_str()))
		{
			LUA_ERROR(lstate_, "Can not parse file " + filename + " ! ");
		}
	}
	inline void ParseString(std::string const & str)
	{
		Init();
		if (luaL_dostring(self_->lstate_.get(), str.c_str()))
		{
			LUA_ERROR(lstate_, "Parsing string error! \n\t" + str);
		}
	}

	inline bool operator==(LuaObject const &r)
	{
		return self_ == r.self_;
	}

	class iterator
	{
		std::shared_ptr<LuaState> parent_;
		std::shared_ptr<LuaState> key_;
		std::shared_ptr<LuaState> value_;
		lua_State *lstate_;

		void Next()
		{
			if (key_ == nullptr)
			{
				lua_pushnil(lstate_);
			}
			else
			{
				lua_pushvalue(lstate_, key_->idx_);
			}

			if (lua_next(lstate_, parent_->idx_))
			{
				int top = lua_gettop(lstate_);

				key_ = std::shared_ptr<LuaState>(
						new LuaState(parent_, top - 1,
								parent_->path_ + "[key]"));

				value_ = std::shared_ptr<LuaState>(
						new LuaState(parent_, top - 0,
								parent_->path_ + "[key]"));

			}
			else
			{
				parent_ = nullptr;
				key_ = nullptr;
				value_ = nullptr;
			}

		}
	public:
		iterator() :
				parent_(nullptr), key_(nullptr), value_(nullptr), lstate_(
						nullptr)
		{

			CHECK("Empty Iterator Construct ");
		}
		iterator(std::shared_ptr<LuaState> p) :
				parent_(p), key_(nullptr), value_(nullptr), lstate_(
						parent_->lstate_.get())
		{
			CHECK("Begin Iterator Construct ");

			if (!lua_istable(lstate_, parent_->idx_))
			{
				LOGIC_ERROR << "Object [" << parent_->path_
						<< "] is not indexable!";
			}
			else
			{
				Next();
			}
			CHECK("Begin Iterator Construct Done ");

		}

		~iterator()
		{
			CHECK("  Iterator Destruct ");

		}

		bool operator!=(iterator const & r) const
		{
			CHECK("Compare NEQ");
			return (r.key_ != key_);
		}
		std::pair<LuaObject, LuaObject> operator*()
		{
			CHECK(key_->idx_);
			if (key_ == nullptr || value_ == nullptr)
			{
				LOGIC_ERROR << "the value of this iterator is invalid!";
			}
			return std::make_pair(LuaObject(key_), LuaObject(value_));
		}

		iterator & operator++()
		{
			Next();
			return *this;
		}
	}
	;

	iterator begin()
	{
		return iterator(self_);
	}
	iterator end()
	{
		return iterator();
	}

	template<typename TFun>
	void ForEach(TFun const &fun)
	{
		if (lua_type(lstate_, self_->idx_) == LUA_TTABLE)
		{
			/* table is in the stack at index 'idx' */
			lua_pushnil(lstate_); /* first key */
			while (lua_next(lstate_, self_->idx_))
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */
				fun(
						LuaObject(
								new LuaState(self_, -2, self_->path_ + "[key]",
										true)),
						LuaObject(
								new LuaState(self_, -1,
										self_->path_ + "[value]", true)));

				lua_pop(lstate_, 1);
			}
		}
	}

	template<typename T>
	inline LuaObject GetChild(T const & key) const
	{
		return at(key);
	}

	inline LuaObject operator[](std::string const & s) const
	{

		if (self_->idx_ == LUA_RIDX_GLOBALS)
		{
			lua_getglobal(lstate_, s.c_str());

		}
		else
		{
			lua_getfield(lstate_, self_->idx_, s.c_str());

		}
		return LuaObject(new LuaState(self_, lua_gettop(lstate_), ToString(s)));
	}

	template<typename T>
	inline LuaObject at(T const & s) const
	{
		LuaObject res = this->operator[](s);

		if (lua_isnil( lstate_ , res.self_->idx_))
		{
			throw(std::out_of_range(
					"\"" + ToString(s) + "\" is not an element in ["
							+ self_->path_ + "] !"));

			res = LuaObject();
		}

		return std::move(res);
	}

	inline LuaObject operator[](int s) const
	{
		lua_rawgeti(lstate_, self_->idx_, s + 1);

		return LuaObject(
				new LuaState(self_, lua_gettop(lstate_),
						"[" + ToString(s) + "]"));
	}

	inline LuaObject at(int s) const
	{
		if (!lua_istable(lstate_, self_->idx_))
		{
			LOGIC_ERROR << "Object [" << self_->path_ << "] is not indexable!";
		}

		if (s > GetLength())
		{
			OUT_RANGE_ERROR << "Object " << self_->path_ << " " << s << ">="
					<< GetLength();

		}
		return std::move(this->operator[](s));
	}
	inline size_t GetLength() const
	{
		return lua_rawlen(lstate_, self_->idx_);
	}
	template<typename ...Args>
	LuaObject operator()(Args const &... args) const
	{
		if (!lua_isfunction(lstate_,self_->idx_))
		{
			LOGIC_ERROR << self_->path_ << " is not  a function!";
		}

		lua_pushvalue(lstate_, self_->idx_);

		ToLua(lstate_, args...);

		lua_call(lstate_, sizeof...(args), 1);

		int idx = lua_gettop(lstate_);

		return LuaObject(new LuaState(self_->lstate_, idx));

	}

	template<typename T>
	inline void GetValue(std::string const & name, T*res) const
	{
		*res = GetChild(name).as<T>();
	}

	template<typename T>
	inline T Get(std::string const & name, T const & default_value = T()) const
	{
		T res;
		try
		{
			res = at(name).as<T>();

		} catch (...)
		{
			res = default_value;
		}

		return std::move(res);
	}

	template<typename T>
	T as() const
	{
		T res;
		FromLua(lstate_, self_->idx_, res);
		return (res);
	}

	template<typename T>
	T as(T const & default_value) const
	{
		T res;
		try
		{
			res = as<T>();

		} catch (...)
		{
			res = default_value;
		}
		return (res);
	}

	template<typename T>
	void SetValue(std::string const &name, T const &v)
	{
		ToLua(lstate_, v);
		lua_setfield(lstate_, self_->idx_, name.c_str());
	}

}
;

#define DEF_LUA_TRANS(_TYPE_,_TO_FUN_,_FROM_FUN_,_CHECK_FUN_)                                     \
template<> struct LuaTrans<_TYPE_>                                                    \
{                                                                                     \
	typedef _TYPE_ value_type;                                                        \
                                                                                      \
	static inline void From(lua_State*L, int idx, value_type & v)                  \
	{                                                                                 \
		if (_CHECK_FUN_(L, idx))                                                     \
		{                                                                             \
			v = _FROM_FUN_(L, idx);                                                   \
		}                                                                             \
		else                                                                          \
		{                                                                             \
			LOGIC_ERROR << "Can not convert type "                                    \
					<< lua_typename(L, lua_type(L, idx))                              \
                    << " to "<<  __STRING(_TYPE_) <<"!";                              \
		}                                                                             \
	}                                                                                 \
	static inline void To(lua_State*L, value_type const & v)                       \
	{                                                                                 \
		_TO_FUN_(L, v);                                                               \
	}                                                                                 \
};                                                                                    \

DEF_LUA_TRANS(double, lua_pushnumber, lua_tonumber, lua_isnumber)
DEF_LUA_TRANS(int, lua_pushinteger, lua_tointeger, lua_isnumber)
DEF_LUA_TRANS(unsigned int, lua_pushunsigned, lua_tounsigned, lua_isnumber)
DEF_LUA_TRANS(long, lua_pushinteger, lua_tointeger, lua_isnumber)
DEF_LUA_TRANS(unsigned long, lua_pushunsigned, lua_tounsigned, lua_isnumber)
DEF_LUA_TRANS(bool, lua_pushboolean, lua_toboolean, lua_isboolean)
#undef DEF_LUA_TRANS

template<> struct LuaTrans<std::string>
{
	typedef std::string value_type;

	static inline void From(lua_State*L, int idx, value_type & v)
	{
		if (lua_isstring(L, idx))
		{
			v = lua_tostring(L, idx);
		}
		else
		{
			LOGIC_ERROR << "Can not convert type "
					<< lua_typename(L, lua_type(L, idx)) << " to double !";
		}
	}
	static inline void To(lua_State*L, value_type const & v)
	{
		lua_pushstring(L, v.c_str());
	}
};

//template<typename T1, typename T2>
//inline void From(int idx, std::pair<T1, T2> *res) const
//{
//	if (lua_istable(lstate_, idx))
//	{
//		int top = lua_gettop(lstate_);
//		if (idx < 0)
//		{
//			idx += top + 1;
//		}
//		lua_rawgeti(lstate_, idx, 1);
//		From(-1, res->first);
//		lua_pop(lstate_, 1);
//		lua_rawgeti(lstate_, idx, 2);
//		From(-1, res->second);
//		lua_pop(lstate_, 1);
//	}
//	else
//	{
//		ERROR << (self_->path_ + " is not a std::pair<T1, T2>");
//	}
//}

template<int N, typename T> struct LuaTrans<nTuple<N, T>>
{
	typedef nTuple<N, T> value_type;

	static inline void From(lua_State*L, int idx, value_type & v)
	{
		if (lua_istable(L, idx))
		{
			size_t num = lua_rawlen(L, idx);
			for (size_t s = 0; s < N; ++s)
			{
				lua_rawgeti(L, idx, s % num + 1);
				FromLua(L, -1, (v[s]));
				lua_pop(L, 1);
			}

		}
		else
		{
			LOGIC_ERROR << "Can not convert type "
					<< lua_typename(L, lua_type(L, idx)) << " to nTuple !";
		}
	}
	static inline void To(lua_State*L, value_type const & v)
	{
		LOGIC_ERROR << " UNIMPLEMENTED!!";
	}
};

template<typename T> struct LuaTrans<std::vector<T> >
{
	typedef std::vector<T> value_type;

	static inline void From(lua_State*L, int idx, value_type & v)
	{
		if (lua_istable(L, idx))
		{
			size_t fnum = lua_rawlen(L, idx);

			if (fnum > 0)
			{
				if (v.size() < fnum)
				{
					v.resize(fnum);
				}
				for (size_t s = 0; s < fnum; ++s)
				{
					lua_rawgeti(L, idx, s % fnum + 1);
					FromLua(L, -1, v[s]);
					lua_pop(L, 1);
				}
			}

		}
		else
		{
			LOGIC_ERROR << "Can not convert type "
					<< lua_typename(L, lua_type(L, idx)) << " to vector !";
		}
	}
	static inline void To(lua_State*L, value_type const & v)
	{
		LOGIC_ERROR << " UNIMPLEMENTED!!";
	}
};
template<typename T> struct LuaTrans<std::list<T> >
{
	typedef std::list<T> value_type;

	static inline void From(lua_State*L, int idx, value_type & v)
	{
		if (lua_istable(L, idx))
		{
			size_t fnum = lua_rawlen(L, idx);

			for (size_t s = 0; s < fnum; ++s)
			{
				lua_rawgeti(L, idx, s % fnum + 1);
				T tmp;
				FromLua(L, -1, tmp);
				v.push_back(tmp);
				lua_pop(L, 1);
			}

		}
		else
		{
			LOGIC_ERROR << "Can not convert type "
					<< lua_typename(L, lua_type(L, idx)) << " to list !";
		}
	}
	static inline void To(lua_State*L, value_type const & v)
	{
		LOGIC_ERROR << " UNIMPLEMENTED!!";
	}
};

template<typename T1, typename T2> struct LuaTrans<std::map<T1, T2> >
{
	typedef std::map<T1, T2> value_type;

	static inline void From(lua_State*L, int idx, value_type & v)
	{
		if (lua_istable(L, idx))
		{
			lua_pushnil(L); /* first key */

			T1 key;
			T2 value;

			while (lua_next(L, idx))
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */

				FromLua(L, -2, &key);
				FromLua(L, -1, &value);
				v[key] = value;
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L, 1);
			}

		}
		else
		{
			LOGIC_ERROR << "Can not convert type "
					<< lua_typename(L, lua_type(L, idx)) << " to map !";
		}
	}
	static inline void To(lua_State*L, value_type const & v)
	{
		LOGIC_ERROR << " UNIMPLEMENTED!!";
	}
};

} // namespace simpla

/*
 *
 *
 * struct LuaStateHolder
 {

 std::shared_ptr<LuaStateHolder> parent_;
 std::string key_;
 int self_->idx_;
 lua_State * lstate_;

 LuaStateHolder() :
 idx_(LUA_GLOBALSINDEX), key_(""), lstate_(luaL_newstate())

 {
 luaL_openlibs(lstate_);
 }
 LuaStateHolder(std::shared_ptr<LuaStateHolder> p, std::string const & key =
 "") :
 parent_(p), idx_(0), key_(key), lstate_(p->lstate_)
 {
 lua_getfield(lstate_, p->idx_, key.c_str());

 idx_ = lua_gettop(lstate_);

 if (lua_isnil(lstate_ , idx_))
 {
 lua_remove(lstate_, idx_);
 idx_ = 0;

 throw std::out_of_range(
 "\"" + key + "\" is not an element in " + p->Path() + "!");
 }
 else if (lua_isfunction(lstate_,idx_))
 {
 lua_remove(lstate_, idx_);
 idx_ = 0;
 }
 else if (!lua_istable(lstate_,idx_))
 {
 lua_remove(lstate_, idx_);
 idx_ = 0;
 throw std::out_of_range(key + " is not a table or function!");
 }
 }

 LuaStateHolder(std::shared_ptr<LuaStateHolder> p, int idx) :
 parent_(p), idx_(idx), key_(""), lstate_(p->lstate_)
 {
 }

 ~LuaStateHolder()
 {

 if (idx_ == LUA_GLOBALSINDEX)
 {
 lua_close(lstate_);
 }
 else if (idx_ != 0)
 {
 lua_remove(lstate_, idx_);
 }

 }

 inline std::string Path() const
 {
 std::string res;
 if (idx_ == LUA_GLOBALSINDEX)
 {
 res = "[root]";
 }
 else
 {
 if (key_ != "")
 {
 res = parent_->key_ + "." + key_;
 }
 else
 {
 //				char tmp[20];
 //				itoa(idx_, tmp,10);
 //				res = parent_->key_ + ".[" + tmp + "]";
 }
 }
 return (res);
 }

 };

 class LuaIterator
 {
 std::shared_ptr<LuaStateHolder> holder_;
 public:

 LuaObject operator*()
 {
 return LuaObject(holder_);
 }
 };
 *
 * */
//template<typename T>
//	inline void From(int idx, T *res)
//	{
//		switch (lua_type(lstate.get(), idx))
//		{
//		case LUA_TBOOLEAN:
//			*res = lua_toboolean(lstate.get(), idx);
//			break;
//		case LUA_TNUMBER:
//			*res = lua_tonumber(lstate.get(), idx);
//			break;
//		case LUA_TTABLE:
//		{
//			//			typedef typename Reference<T>::KeyType KeyType;
//			//			typedef typename Reference<T>::ValueType ValueType;
//			//
//			//			/* table is in the stack at index 'idx' */
//			//			lua_pushnil(lstate.get()); /* first key */
//			//			ValueType item;
//			//			KeyType key;
//			//			while (lua_next(lstate.get(), -2))
//			//			{
//			//				/* uses 'key' (at index -2) and 'value' (at index -1) */
//			//				From(-1, item);
//			//				From(-2, key);
//			//				Reference<T>::index(res, key) = item;
//			//				/* removes 'value'; keeps 'key' for next iteration */
//			//				lua_pop(lstate.get(), 1);
//			//
//			//			}
//			break;
//		}
//		}
//	}
//	template<typename T>
//	inline void getExprTo(std::string const & expr, T * v)
//	{
//		std::string e = std::string("__evalExpr=") + expr;
//
//		if (luaL_dostring(lstate.get(), e.c_str()))
//		{
//			LUA_ERROR(lstate.get(), e);
//		}
//
//		GetValue2("__evalExpr", v);
//	}
//
//	template<typename T>
//	inline void getExprToArray(std::string const & expr, std::shared_ptr<T> v)
//	{
//		std::string e = std::string("__evalExpr=") + expr;
//
//		if (luaL_dostring(lstate.get(), e.c_str()))
//		{
//			LUA_ERROR(lstate.get(), e);
//		}
//
//		fillArray2("__evalExpr", v);
//	}
//	template<typename T>
//	inline void fillArray(std::string const& key, T & array)
//	{
//		lua_getfield(lstate.get(), LUA_GLOBALSINDEX, key.c_str());
//		int idx = lua_gettop(lstate.get());
//		try
//		{
//			lua_fillArray(idx, array, 0);
//		} catch (std::string const & e)
//		{
//			ERROR << ("Can not parse \"" + key + "\" to " + e + " !");
//		}
//		lua_pop(lstate.get(), 1);
//
//	}
//	template<typename T>
//	inline void fillArray2(std::string const & key, std::shared_ptr<T> array)
//	{
//		fillArray(key, *array);
//	}
#endif  // INCLUDE_LUA_PARSER_H_
