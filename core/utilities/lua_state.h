/*Copyright (C) 2007-2011 YU Zhi. All rights reserved.
 *
 * @file lua_state.h
 *
 * @date 2010-9-22
 * @author salmon
 */

#ifndef INCLUDE_LUA_PARSER_H_
#define INCLUDE_LUA_PARSER_H_

extern "C"
{

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM < 502
#error  need lua version >502
#endif
}
#include <algorithm>
#include <complex>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <tuple>

#include "log.h"
#include "utilities.h"
#include "ntuple.h"
#include "any.h"

namespace simpla
{
/**
 *  \defgroup  Configure  Configuration
 *  @{
 *  \defgroup  EmScript Embaded Scirpt
 *   @{
 *      \defgroup  Convert Convert
 *       \brief Transfer data  between different lanuage
 *      \defgroup  Lua Lua
 *   @}
 *  @}
 */

//! \ingroup Lua
#define LUA_ERROR(_L, _MSG_)  Logger(LOG_ERROR)<<"[Lua error]"<<(_MSG_)<<std::string("\n") << lua_tostring(_L, -1) ; lua_pop(_L, 1);throw(std::runtime_error(""));

class LuaIterator;
class LuaObject;

template<typename T> struct LuaTrans;

inline unsigned int ToLua(std::shared_ptr<lua_State> L)
{
	return 0;
}
template<typename T, typename ... Args>
inline unsigned int ToLua(std::shared_ptr<lua_State> L, T const & v, Args const & ... rest)
{
	return LuaTrans<T>::To(L, v) + ToLua(L, rest...);
}

inline unsigned int FromLua(std::shared_ptr<lua_State> L, int)
{
	return 0;
}

template<typename T, typename ... Args>
inline unsigned int FromLua(std::shared_ptr<lua_State> L, unsigned int idx, T * v, Args * ... rest)
{
	return LuaTrans<T>::From(L, idx, v) + FromLua(L, idx + 1, rest...);
}

/**
 *
 *  \ingroup   Lua
 *  @class LuaObject
 *  \brief interface to Lua Script
 */
class LuaObject
{
	std::shared_ptr<lua_State> L_;

	int GLOBAL_REF_IDX_;
	int self_;
	std::string path_;

public:

	typedef LuaObject this_type;

	LuaObject()
			: L_(nullptr), self_(0), GLOBAL_REF_IDX_(0)

	{
	}

	LuaObject(std::shared_ptr<lua_State> l, unsigned int G, unsigned int s, std::string const & path = "")
			: L_(l), GLOBAL_REF_IDX_(G), self_(s), path_(path)
	{
	}
	LuaObject(LuaObject const & r)
			: L_(r.L_), GLOBAL_REF_IDX_(r.GLOBAL_REF_IDX_), path_(r.path_)
	{

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, r.self_);
		self_ = luaL_ref(L_.get(), GLOBAL_REF_IDX_);
	}

	LuaObject(LuaObject && r)
			: L_(r.L_), GLOBAL_REF_IDX_(r.GLOBAL_REF_IDX_), self_(r.self_), path_(r.path_)
	{
		r.self_ = 0;
	}

	LuaObject & operator=(LuaObject const & r)
	{
		this->L_ = r.L_;
		this->GLOBAL_REF_IDX_ = r.GLOBAL_REF_IDX_;
		this->path_ = r.path_;
		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, r.self_);
		self_ = luaL_ref(L_.get(), GLOBAL_REF_IDX_);
		return *this;
	}

	~LuaObject()
	{

		if (self_ > 0)
		{
			luaL_unref(L_.get(), GLOBAL_REF_IDX_, self_);
		}

		if (L_.unique())
		{
			lua_remove(L_.get(), GLOBAL_REF_IDX_);
		}
	}

	inline std::basic_ostream<char> & Serialize(std::basic_ostream<char> &os)
	{
		int top = lua_gettop(L_.get());
		for (int i = 1; i < top; ++i)
		{
			int t = lua_type(L_.get(), i);
			switch (t)
			{
			case LUA_TSTRING:
				os << "[" << i << "]=" << lua_tostring(L_.get(), i) << std::endl;
				break;

			case LUA_TBOOLEAN:
				os << "[" << i << "]=" << std::boolalpha << lua_toboolean(L_.get(), i) << std::endl;
				break;

			case LUA_TNUMBER:
				os << "[" << i << "]=" << lua_tonumber(L_.get(), i) << std::endl;
				break;
			case LUA_TTABLE:
				os << "[" << i << "]=" << "is a table" << std::endl;
				break;
			default:
				os << "[" << i << "]=" << "is an unknown type" << std::endl;
			}
		}
		os << "--  End the listing --" << std::endl;

		return os;
	}

	inline bool IsNull() const
	{
		return L_ == nullptr;
	}
	inline bool empty() const // STL style
	{
		return L_ == nullptr;
	}

	operator bool() const
	{
		return L_ != nullptr;
	}

#define DEF_TYPE_CHECK(_FUN_NAME_,_LUA_FUN_)                              \
	inline bool _FUN_NAME_() const                                        \
	{   bool res=false;                                                   \
	    if(L_!=nullptr)                                                   \
	    {                                                                 \
		  lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);                  \
		   res = _LUA_FUN_(L_.get(), -1);                                 \
		  lua_pop(L_.get(), 1);                                           \
	    }                                                                 \
		return res;                                                       \
	}

	DEF_TYPE_CHECK(is_nil,lua_isnil)
	DEF_TYPE_CHECK(is_number,lua_isnumber)
	DEF_TYPE_CHECK(is_string,lua_isstring)
	DEF_TYPE_CHECK(is_boolean,lua_isboolean)
	DEF_TYPE_CHECK(is_lightuserdata,lua_islightuserdata)
	DEF_TYPE_CHECK(is_function,lua_isfunction)
	DEF_TYPE_CHECK(is_thread,lua_isthread)
	DEF_TYPE_CHECK(is_table,lua_istable)
#undef DEF_TYPE_CHECK

	inline std::string get_typename() const
	{
		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		std::string res = lua_typename(L_.get(), -1);
		lua_pop(L_.get(), 1);
		return res;
	}

	void init()
	{
		if (self_ == 0)
		{
			L_ = std::shared_ptr<lua_State>(luaL_newstate(), lua_close);

			luaL_openlibs(L_.get());

			lua_newtable(L_.get());  // new table on stack

			GLOBAL_REF_IDX_ = lua_gettop(L_.get());

			self_ = -1;

			path_ = "<GLOBAL>";

		}
	}

	inline void ParseFile(std::string const & filename)
	{
		init();
		if (filename != "" && luaL_dofile(L_.get(), filename.c_str()))
		{
			LUA_ERROR(L_.get(), "Can not parse file " + filename + " ! ");
		}
	}
	inline void ParseString(std::string const & str)
	{
		init();
		if (luaL_dostring(L_.get(), str.c_str()))
		{
			LUA_ERROR(L_.get(), "Parsing string error! \n\t" + str);
		}
	}

	class iterator
	{
		std::shared_ptr<lua_State> L_;
		int GLOBAL_IDX_;
		int parent_;
		int key_;
		int value_;
		std::string path_;
	public:
		void Next()
		{
			lua_rawgeti(L_.get(), GLOBAL_IDX_, parent_);

			int tidx = lua_gettop(L_.get());

			if (lua_isnil(L_.get(), tidx))
			{
				LOGIC_ERROR(path_ + " is not iteraterable!");
			}

			if (key_ == LUA_NOREF)
			{
				lua_pushnil(L_.get());
			}
			else
			{
				lua_rawgeti(L_.get(), GLOBAL_IDX_, key_);
			}

			int v, k;

			if (lua_next(L_.get(), tidx))
			{
				v = luaL_ref(L_.get(), GLOBAL_IDX_);
				k = luaL_ref(L_.get(), GLOBAL_IDX_);
			}
			else
			{
				k = LUA_NOREF;
				v = LUA_NOREF;
			}
			if (key_ != LUA_NOREF)
				luaL_unref(L_.get(), GLOBAL_IDX_, key_);
			if (value_ != LUA_NOREF)
				luaL_unref(L_.get(), GLOBAL_IDX_, value_);

			key_ = k;
			value_ = v;

			lua_pop(L_.get(), 1);
		}
	public:
		iterator()
				: L_(nullptr), GLOBAL_IDX_(0), parent_( LUA_NOREF), key_(
				LUA_NOREF), value_( LUA_NOREF)
		{

		}
		iterator(iterator const& r)
				: L_(r.L_), GLOBAL_IDX_(r.GLOBAL_IDX_)
		{

			lua_rawgeti(L_.get(), GLOBAL_IDX_, r.parent_);

			parent_ = luaL_ref(L_.get(), GLOBAL_IDX_);

			lua_rawgeti(L_.get(), GLOBAL_IDX_, r.key_);

			key_ = luaL_ref(L_.get(), GLOBAL_IDX_);

			lua_rawgeti(L_.get(), GLOBAL_IDX_, r.value_);

			value_ = luaL_ref(L_.get(), GLOBAL_IDX_);

		}
		iterator(iterator && r)
				: L_(r.L_), GLOBAL_IDX_(r.GLOBAL_IDX_), parent_(r.parent_), key_(r.key_), value_(r.value_)
		{
			r.parent_ = LUA_NOREF;
			r.key_ = LUA_NOREF;
			r.value_ = LUA_NOREF;
		}
		iterator(std::shared_ptr<lua_State> L, unsigned int G, unsigned int p, std::string path)
				: L_(L), GLOBAL_IDX_(G), parent_(p), key_(LUA_NOREF), value_(
				LUA_NOREF), path_(path + "[iterator]")
		{
			lua_rawgeti(L_.get(), GLOBAL_IDX_, p);
			bool is_table = lua_istable(L_.get(), -1);
			parent_ = luaL_ref(L_.get(), GLOBAL_IDX_);

			if (!is_table)
			{
				LOGIC_ERROR("Object is not indexable!");
			}
			else
			{
				Next();
			}

		}

		~iterator()
		{
			if (key_ != LUA_NOREF)
			{
				luaL_unref(L_.get(), GLOBAL_IDX_, key_);
			}
			if (value_ != LUA_NOREF)
			{
				luaL_unref(L_.get(), GLOBAL_IDX_, value_);
			}
			if (parent_ != LUA_NOREF)
			{
				luaL_unref(L_.get(), GLOBAL_IDX_, parent_);
			}
			if (L_.unique())
			{
				lua_remove(L_.get(), GLOBAL_IDX_);
			}
//			if (L_ != nullptr)
//				CHECK(lua_rawlen(L_.get(), GLOBAL_IDX_));
		}

		bool operator!=(iterator const & r) const
		{
			return (r.key_ != key_);
		}
		std::pair<LuaObject, LuaObject> operator*()
		{
			if (key_ == LUA_NOREF || value_ == LUA_NOREF)
			{
				LOGIC_ERROR("the value of this iterator is invalid!");
			}

			lua_rawgeti(L_.get(), GLOBAL_IDX_, key_);

			int key = luaL_ref(L_.get(), GLOBAL_IDX_);

			lua_rawgeti(L_.get(), GLOBAL_IDX_, value_);

			int value = luaL_ref(L_.get(), GLOBAL_IDX_);

			return std::make_pair(LuaObject(L_, GLOBAL_IDX_, key, path_ + ".key"),
			        LuaObject(L_, GLOBAL_IDX_, value, path_ + ".value"));
		}

		iterator & operator++()
		{
			Next();
			return *this;
		}
	};

	iterator begin()
	{
		if (empty())
			return end();
		else
			return iterator(L_, GLOBAL_REF_IDX_, self_, path_);
	}
	iterator end()
	{
		return iterator();
	}
	iterator begin() const
	{
		return iterator(L_, GLOBAL_REF_IDX_, self_, path_);
	}
	iterator end() const
	{
		return iterator();
	}

	template<typename T> inline LuaObject get_child(T const & key) const
	{
		if (IsNull())
			return LuaObject();

		return std::move(at(key));
	}

	size_t size() const
	{
		if (IsNull())
			return 0;

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		size_t res = lua_rawlen(L_.get(), -1);
		lua_pop(L_.get(), 1);
		return std::move(res);
	}

	inline LuaObject operator[](char const s[]) const noexcept
	{
		return operator[](std::string(s));
	}
	inline LuaObject operator[](std::string const & s) const noexcept
	{
		if (IsNull())
			return LuaObject();

		bool is_global = (self_ < 0);
		if (is_global)
		{
			lua_getglobal(L_.get(), s.c_str());
		}
		else
		{

			lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
			lua_getfield(L_.get(), -1, s.c_str());
		}

		if (lua_isnil(L_.get(), lua_gettop(L_.get())))
		{
			lua_pop(L_.get(), 1);
			return std::move(LuaObject());
		}
		else
		{

			int id = luaL_ref(L_.get(), GLOBAL_REF_IDX_);

			if (!is_global)
			{
				lua_pop(L_.get(), 1);
			}

			return std::move(LuaObject(L_, GLOBAL_REF_IDX_, id, path_ + "." + ToString(s)));
		}
	}

	//! unsafe fast access, no boundary check, no path information
	inline LuaObject operator[](int s) const noexcept
	{
		if (IsNull())
			return LuaObject();

		if (self_ < 0 || L_ == nullptr)
		{
			LOGIC_ERROR(path_ + " is not indexable!");
		}
		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		int tidx = lua_gettop(L_.get());
		lua_rawgeti(L_.get(), tidx, s + 1);
		int res = luaL_ref(L_.get(), GLOBAL_REF_IDX_);
		lua_pop(L_.get(), 1);

		return std::move(LuaObject(L_, GLOBAL_REF_IDX_, res));

	}

	//! index operator with out_of_range exception
	template<typename TIDX> inline LuaObject at(TIDX const & s) const
	{
		if (IsNull())
			return LuaObject();
		LuaObject res = this->operator[](s);
		if (res.IsNull())
		{

			throw(std::out_of_range(ToString(s) + "\" is not an element in " + path_));
		}

		return std::move(res);

	}

	//! safe access, with boundary check, no path information
	inline LuaObject at(int s) const
	{
		if (IsNull())
			return LuaObject();

		if (self_ < 0 || L_ == nullptr)
		{
			LOGIC_ERROR(path_ + " is not indexable!");
		}

		if (s > size())
		{
			throw(std::out_of_range(
			        "index out of range! " + path_ + "[" + ToString(s) + " > " + ToString(size()) + " ]"));
		}

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		int tidx = lua_gettop(L_.get());
		lua_rawgeti(L_.get(), tidx, s + 1);
		int res = luaL_ref(L_.get(), GLOBAL_REF_IDX_);
		lua_pop(L_.get(), 1);

		return std::move(LuaObject(L_, GLOBAL_REF_IDX_, res, path_ + "[" + ToString(s) + "]"));

	}

	template<typename ...Args> LuaObject operator()(Args const &... args) const
	{
		if (IsNull())
			return LuaObject();

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);

		int idx = lua_gettop(L_.get());

		if (!lua_isfunction(L_.get(), idx))
		{
			LOGIC_ERROR(path_ + " is not  a function!");
		}

		lua_pcall(L_.get(), ToLua(L_, args...), 1, 0);

		return LuaObject(L_, GLOBAL_REF_IDX_, luaL_ref(L_.get(), GLOBAL_REF_IDX_), path_ + "[ret]");

	}

	template<typename T, typename ...Args> inline T create_object(Args && ... args) const
	{
		if (IsNull())
		{
			return std::move(T());
		}
		else
		{
			return std::move(T(*this, std::forward<Args>(args)...));
		}

	}

	template<typename T> inline T as() const
	{
		T res;
		as(&res);
		return std::move(res);
	}

	template<typename ... T> inline std::tuple<T...> as_tuple() const
	{
		return std::move(as<std::tuple<T...>>());
	}

	template<typename TRect, typename ...Args> void as(std::function<TRect(Args ...)> *res) const
	{

		if (is_number() || is_table())
		{
			auto value = this->as<TRect>();

			*res = [value](Args ...args )->TRect
			{	return value;};

		}
		else if (is_function())
		{
			LuaObject obj = *this;
			*res = [obj](Args ...args)->TRect
			{	return obj(std::forward<Args>(args)...).template as<TRect>();};
		}

	}

	template<typename T> inline T as(T const &default_value) const
	{
		T res = default_value;
		as(&res);
		return (res);
	}

	template<typename T> inline void as(T* res) const
	{
		if (!IsNull())
		{
			lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
			FromLua(L_, lua_gettop(L_.get()), res);
			lua_pop(L_.get(), 1);
		}
	}

//	template<typename T> inline T get(std::string const & name, T const & default_value = T()) const noexcept
//	{
//		LuaObject res = this->operator[](name);
//
//		if (res.IsNull())
//		{
//			return default_value;
//		}
//		else
//		{
//
//			return std::move(res.as<T>());
//		}
//	}
//
//	template<typename T> inline void GetValue(std::string const & name, T *v) const noexcept
//	{
//		operator[](name).as(v);
//	}
//
//	template<typename T> inline void GetValue(int s, T *v) const noexcept
//	{
//		operator[](s).as(v);
//	}

	template<typename T> inline void set(std::string const & name, T const &v) const
	{
		if (IsNull())
			return;

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		ToLua(L_.get(), v);
		LuaTrans<T>::To(L_.get(), v);
		lua_setfield(L_.get(), -2, name.c_str());
		lua_pop(L_.get(), 1);
	}

	template<typename T> inline void set(int s, T const &v) const
	{
		if (IsNull())
			return;

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		ToLua(L_.get(), v);
		lua_rawseti(L_.get(), -2, s);
		lua_pop(L_.get(), 1);
	}

	template<typename T> inline void add(T const &v) const
	{
		if (IsNull())
			return;

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		ToLua(L_.get(), v);
		size_t len = lua_rawlen(L_.get(), -1);
		lua_rawseti(L_.get(), -2, len + 1);
		lua_pop(L_.get(), 1);
	}

	/**
	 *
	 * @param name the field name of table ,if name=="" use lua_settable, else append
	 *        new table to the end of parent table
	 * @param narr is a hint for how many elements the table will have as a sequence;
	 * @param nrec is a hint for how many other elements the table will have.
	 * @return a LuaObject of new table
	 *
	 * Lua may use these hints to preallocate memory for the new table.
	 *  This pre-allocation is useful for performance when you know in advance how
	 *   many elements the table will have.
	 *
	 *  \note Lua.org:createtable
	 */
	inline LuaObject new_table(std::string const & name, unsigned int narr = 0, unsigned int nrec = 0)
	{
		if (IsNull())
			return LuaObject();

		lua_rawgeti(L_.get(), GLOBAL_REF_IDX_, self_);
		int tidx = lua_gettop(L_.get());
		lua_createtable(L_.get(), narr, nrec);
		if (name == "")
		{
			int len = lua_rawlen(L_.get(), tidx);
			lua_rawseti(L_.get(), tidx, len + 1);
			lua_rawgeti(L_.get(), tidx, len + 1);
		}
		else
		{
			lua_setfield(L_.get(), tidx, name.c_str());
			lua_getfield(L_.get(), tidx, name.c_str());
		}
		LuaObject res(L_, GLOBAL_REF_IDX_, luaL_ref(L_.get(), GLOBAL_REF_IDX_), path_ + "." + name);
		lua_pop(L_.get(), 1);
		return std::move(res);
	}
};

/**
 *
 *     \ingroup Convert
 *     @{
 */
#define DEF_LUA_TRANS(_TYPE_,_TO_FUN_,_FROM_FUN_,_CHECK_FUN_)                                     \
template<> struct LuaTrans<_TYPE_>                                                    \
{                                                                                     \
	typedef _TYPE_ value_type;                                                        \
                                                                                      \
	static inline  unsigned int  From(std::shared_ptr<lua_State>L,  unsigned int  idx, value_type * v,                    \
            value_type const &default_value=value_type())                            \
	{                                                                                 \
		if (_CHECK_FUN_(L.get(), idx))                                                     \
		{                                                                             \
			*v = _FROM_FUN_(L.get(), idx);                                                   \
		}                                                                             \
		else                                                                          \
		{   *v = default_value;                                                       \
		}                                                                             \
        return 1;                                                                     \
	}                                                                                 \
	static inline  unsigned int  To(std::shared_ptr<lua_State>L, value_type const & v)                       \
	{                                                                                 \
		_TO_FUN_(L.get(), v);return 1;                                                \
	}                                                                                 \
};                                                                                    \

DEF_LUA_TRANS(double, lua_pushnumber, lua_tonumber, lua_isnumber)
DEF_LUA_TRANS(int, lua_pushinteger, lua_tointeger, lua_isnumber)
DEF_LUA_TRANS(unsigned int, lua_pushunsigned, lua_tounsigned, lua_isnumber)
DEF_LUA_TRANS(long, lua_pushinteger, lua_tointeger, lua_isnumber)
DEF_LUA_TRANS(unsigned long, lua_pushunsigned, lua_tounsigned, lua_isnumber)
DEF_LUA_TRANS(bool, lua_pushboolean, lua_toboolean, lua_isboolean)
#undef DEF_LUA_TRANS

template<typename T> struct LuaTrans
{
	typedef std::string value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_isstring(L.get(), idx))
		{
			*v = lua_tostring(L.get(), idx);
		}
		else
		{
			*v = default_value;
//			LOGIC_ERROR << "Can not convert type "
//					<< lua_typename(L, lua_type(L, idx)) << " to double !";
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		lua_pushstring(L.get(), v.c_str());
		return 1;
	}
};

template<> struct LuaTrans<std::string>
{
	typedef std::string value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_isstring(L.get(), idx))
		{
			*v = lua_tostring(L.get(), idx);
		}
		else
		{
			*v = default_value;
//			LOGIC_ERROR << "Can not convert type "
//					<< lua_typename(L, lua_type(L, idx)) << " to double !";
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		lua_pushstring(L.get(), v.c_str());
		return 1;
	}
};

template<unsigned int N, typename T> struct LuaTrans<nTuple<T,N>>
{
	typedef nTuple<T,N> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			size_t num = lua_rawlen(L.get(), idx);
			for (size_t s = 0; s < N; ++s)
			{
				lua_rawgeti(L.get(), idx, s % num + 1);
				FromLua(L, -1, &((*v)[s]));
				lua_pop(L.get(), 1);
			}

		}
		else
		{
			*v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		lua_newtable(L.get());

		for (int i = 0; i < N; ++i)
		{
			lua_pushinteger(L.get(), i);
			LuaTrans<T>::To(L, v[i]);
			lua_settable(L.get(), -3);
		}
		return 1;

	}
};

template<typename TC> void PushContainer(std::shared_ptr<lua_State> L, TC const & v)
{
	lua_newtable(L.get());

	size_t s = 0;
	for (auto const & vv : v)
	{
		lua_pushinteger(L.get(), s);
		LuaTrans<decltype(vv)>::To(L, vv);
		lua_settable(L.get(), -3);
		++s;
	}
}

template<typename T> struct LuaTrans<std::vector<T> >
{
	typedef std::vector<T> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			size_t fnum = lua_rawlen(L.get(), idx);

			if (fnum > 0)
			{

				for (size_t s = 0; s < fnum; ++s)
				{
					T res;
					lua_rawgeti(L.get(), idx, s % fnum + 1);
					FromLua(L, -1, &(res));
					lua_pop(L.get(), 1);
					v->emplace_back(res);
				}
			}

		}
		else
		{
			*v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		PushContainer(L, v);
		return 1;
	}
};
template<typename T> struct LuaTrans<std::list<T> >
{
	typedef std::list<T> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			size_t fnum = lua_rawlen(L.get(), idx);

			for (size_t s = 0; s < fnum; ++s)
			{
				lua_rawgeti(L.get(), idx, s % fnum + 1);
				T tmp;
				FromLua(L, -1, tmp);
				v->push_back(tmp);
				lua_pop(L.get(), 1);
			}

		}
		else
		{
			v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		PushContainer(L, v);
		return 1;
	}
};

template<typename T1, typename T2> struct LuaTrans<std::map<T1, T2> >
{
	typedef std::map<T1, T2> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			lua_pushnil(L.get()); /* first key */

			T1 key;
			T2 value;

			while (lua_next(L.get(), idx))
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */

				FromLua(L, -2, &key);
				FromLua(L, -1, &value);
				(*v)[key] = value;
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L.get(), 1);
			}

		}
		else
		{
			v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		lua_newtable(L.get());

		for (auto const & vv : v)
		{
			LuaTrans<T1>::To(L, vv.first);
			LuaTrans<T2>::To(L, vv.second);
			lua_settable(L.get(), -3);
		}
		return 1;
	}
};

template<typename T> struct LuaTrans<std::complex<T> >
{
	typedef std::complex<T> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			lua_pushnil(L.get()); /* first key */
			while (lua_next(L.get(), idx))
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */
				T r, i;
				FromLua(L, -2, &r);
				FromLua(L, -1, &i);
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L.get(), 1);

				*v = std::complex<T>(r, i);
			}

		}
		else if (lua_isnumber(L.get(), idx))
		{
			T r;
			FromLua(L, idx, &r);
			*v = std::complex<T>(r, 0);
		}
		else
		{
			*v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		LuaTrans<int>::To(L, 0);
		LuaTrans<T>::To(L, v.real());
		lua_settable(L.get(), -3);
		LuaTrans<int>::To(L, 1);
		LuaTrans<T>::To(L, v.imag());
		lua_settable(L.get(), -3);

		return 1;
	}
};

template<typename T1, typename T2> struct LuaTrans<std::pair<T1, T2> >
{
	typedef std::pair<T1, T2> value_type;

	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v,
	        value_type const &default_value = value_type())
	{
		if (lua_istable(L.get(), idx))
		{
			lua_pushnil(L.get()); /* first key */
			while (lua_next(L.get(), idx))
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */

				FromLua(L, -2, &(v->first));
				FromLua(L, -1, &(v->second));
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L.get(), 1);
			}

		}
		else
		{
			*v = default_value;
		}
		return 1;
	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		LuaTrans<T1>::To(L, v.first);
		LuaTrans<T2>::To(L, v.second);
		lua_settable(L.get(), -3);
		return 1;
	}
};

template<typename ... T> struct LuaTrans<std::tuple<T...> >
{
	typedef std::tuple<T...> value_type;

private:
	static inline unsigned int From_(std::shared_ptr<lua_State> L, unsigned int idx, value_type* v,
	        std::integral_constant<unsigned int, 0>)
	{
		return 0;
	}

	template<unsigned int N>
	static inline unsigned int From_(std::shared_ptr<lua_State> L, unsigned int idx, value_type* v,
	        std::integral_constant<unsigned int, N>)
	{
		lua_rawgeti(L.get(), idx, N); // lua table's index starts from 1
		auto num = FromLua(L, -1, &std::get<N - 1>(*v)); // C++ tuple index start from 0
		lua_pop(L.get(), 1);

		return num + From_(L, idx, v, std::integral_constant<unsigned int, N - 1>());
	}
	static inline unsigned int To_(std::shared_ptr<lua_State> L, value_type const& v,
	        std::integral_constant<unsigned int, 0>)
	{
		return 0;
	}
	template<unsigned int N> static inline unsigned int To_(std::shared_ptr<lua_State> L, value_type const& v,
	        std::integral_constant<unsigned int, N>)
	{
		return ToLua(L, std::get<sizeof...(T) - N >(v)) + To_(L, v, std::integral_constant< unsigned int , N - 1>());
	}
public:
	static inline unsigned int From(std::shared_ptr<lua_State> L, unsigned int idx, value_type * v, value_type const &default_value =
	value_type())
	{
		return From_(L, idx, v, std::integral_constant< unsigned int , sizeof...(T)>());

	}
	static inline unsigned int To(std::shared_ptr<lua_State> L, value_type const & v)
	{
		return To_(L, v,std::integral_constant< unsigned int , sizeof...(T)>());
	}

};

/** @} LuaTrans */

template<typename TV> inline TV TypeCast(LuaObject const & obj)
{
	return std::move(obj.as<TV>());
}
inline std::ostream & operator<<(std::ostream & os, LuaObject const & obj)
{
	os << obj.as<std::string>();
	return os;
}

} // namespace simpla

#endif  // INCLUDE_LUA_PARSER_H_
