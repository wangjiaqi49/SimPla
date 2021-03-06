/*
 * cache.h
 *
 *  created on: 2013-11-12
 *      Author: salmon
 */

#ifndef CACHE_H_
#define CACHE_H_
#include "../utilities/sp_type_traits.h"
namespace simpla
{

/**
 * \brief  Cache is transparent, and would not change the behavior
 * of source object. In default, Cache do nothing. It only affect
 * the object by the template specialization of object.
 *
 *
 *
 */
template<typename T>
struct Cache
{
	T & f_;

	template<typename ... Args>
	Cache(T& f, Args ...)
			: f_(f)
	{

	}

	T & operator*()
	{
		return f_;
	}
};

template<typename T>
struct Cache<T const &>
{
	T const & f_;

	template<typename ... Args>
	Cache(T const & f, Args ...)
			: f_(f)
	{

	}

	T const & operator*() const
	{
		return f_;
	}
};

template<typename T>
struct Cache<T*>
{
	T * f_;

	template<typename ... Args>
	Cache(T* f, Args ...)
			: f_(f)
	{
	}

	T & operator*()
	{
		return *f_;
	}
};

template<typename TIndexType, typename T>
void RefreshCache(TIndexType s, T &)
{
}
template<typename TIndexType, typename T>
void RefreshCache(TIndexType s, T *)
{
}
template<typename TIndexType, typename T>
void RefreshCache(TIndexType s, Cache<T> & c)
{
	RefreshCache(s, *c);
}

template<typename TIndexType, typename T, typename ...Others>
void RefreshCache(TIndexType s, T & f, Others & ...others)
{
	RefreshCache(s, f);
	RefreshCache(s, others...);
}

template<typename T>
void FlushCache(T &)
{
}
template<typename T>
void FlushCache(T *f)
{
	FlushCache(*f);
}
template<typename T, typename ...Others>
void FlushCache(T & f, Others & ...others)
{
	FlushCache(f);
	FlushCache(others...);
}

}  // namespace simpla

#endif /* CACHE_H_ */
