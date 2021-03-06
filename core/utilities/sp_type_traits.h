/*
 * sp_type_traits.h
 *
 *  created on: 2014-6-15
 *      Author: salmon
 */

#ifndef SP_TYPE_TRAITS_H_
#define SP_TYPE_TRAITS_H_

#include <type_traits>
#include <memory>
#include <tuple>
#include <utility>
#include <complex>
namespace simpla
{

typedef std::nullptr_t NullType;

struct EmptyType
{
};

#define HAS_MEMBER(_NAME_)                                                                 \
template<typename _T>                                                                      \
struct has_member_##_NAME_                                                                 \
{                                                                                          \
private:                                                                                   \
	typedef std::true_type yes;                                                            \
	typedef std::false_type no;                                                            \
                                                                                           \
	template<typename U>                                                                   \
	static auto test(int) ->  decltype(std::declval<U>()._NAME_  )   ;                     \
	template<typename > static no test(...);                                               \
                                                                                           \
public:                                                                                    \
	static constexpr bool value = !std::is_same<decltype(test< _T>(0)), no>::value;        \
};

#define HAS_TYPE_MEMBER(_NAME_)                                                            \
template<typename _T,typename _D>                                                          \
struct has_type_member_##_NAME_                                                            \
{                                                                                          \
private:                                                                                   \
	typedef std::true_type yes;                                                            \
	typedef std::false_type no;                                                            \
                                                                                           \
	template<typename U>                                                                   \
	static auto test(int) ->  U::_NAME_   ;                     \
	template<typename > static no test(...);                                               \
                                                                                           \
public:                                                                                    \
	static constexpr bool value = std::is_same<decltype(test< _T>(0)), _D>::value;         \
};                                                                                         \
                                                                                           \
template<typename _T, typename _D>                                                         \
typename std::enable_if<has_type_member_##_NAME_<_T, _D>::value, _D>::type                 \
get_member_##_NAME_(_T const & c, _D const & def){	return c._NAME_; }                     \
template<typename _T, typename _D>                                                         \
typename std::enable_if<!has_type_member_##_NAME_<_T, _D>::value, _D>::type                \
get_member_##_NAME_(_T const & c, _D const & def){	return def;}                           \


#define HAS_MEMBER_FUNCTION(_NAME_)                                                                   \
template<typename _T, typename ..._Args>                                                                \
struct has_member_function_##_NAME_                                                                    \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< sizeof...(_Args)==0,                                                      \
	decltype(std::declval<U>()._NAME_() )>::type;                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< ( sizeof...(_Args) >0),                                                   \
	decltype(std::declval<U>()._NAME_(std::declval<_Args>()...) )>::type;                            \
                                                                                                      \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test< _T>(0)), no>::value;                     \
};

#define HAS_CONST_MEMBER_FUNCTION(_NAME_)                                                                   \
template<typename _T, typename ..._Args>                                                                \
struct has_const_member_function_##_NAME_                                                                    \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< sizeof...(_Args)==0,                                                      \
	decltype(std::declval<const U>()._NAME_() )>::type;                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< ( sizeof...(_Args) >0),                                                   \
	decltype(std::declval<const U>()._NAME_(std::declval<_Args>()...) )>::type;                            \
                                                                                                      \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test< _T>(0)), no>::value;                     \
};

#define HAS_STATIC_MEMBER_FUNCTION(_NAME_)                                                                   \
template<typename _T, typename ..._Args>                                                                \
struct has_static_member_function_##_NAME_                                                                    \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< sizeof...(_Args)==0,                                                      \
	decltype(U::_NAME_() )>::type;                                                       \
                                                                                                      \
	template<typename U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< ( sizeof...(_Args) >0),                                                   \
	decltype(U::_NAME_(std::declval<_Args>()...) )>::type;                            \
                                                                                                      \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test< _T>(0)), no>::value;                     \
};

#define HAS_FUNCTION(_NAME_)                                                                   \
template< typename ..._Args>                                                                \
struct has_function_##_NAME_                                                                    \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	static auto test(int) ->                                                                          \
	typename std::enable_if< sizeof...(_Args)==0,                                                      \
	decltype(_NAME_() )>::type;                                                       \
                                                                                                      \
	static auto test(int) ->                                                                          \
	typename std::enable_if< ( sizeof...(_Args) >0),                                                   \
	decltype(_NAME_(std::declval<_Args>()...) )>::type;                            \
                                                                                                      \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test(0)), no>::value;                     \
};

#define HAS_OPERATOR(_NAME_,_OP_)                                                                   \
template<typename _T, typename ... _Args>                                                                \
struct has_operator_##_NAME_                                                                    \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	template<typename _U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< sizeof...(_Args)==0,                                                      \
	decltype(std::declval<_U>().operator _OP_() )>::type;                                                       \
                                                                                                      \
	template<typename _U>                                                                              \
	static auto test(int) ->                                                                          \
	typename std::enable_if< ( sizeof...(_Args) >0),                                                   \
	decltype(std::declval<_U>().operator _OP_(std::declval<_Args>()...) )>::type;                            \
                                                                                                      \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test<_T>(0)), no>::value;                     \
};

#define HAS_TYPE(_NAME_)                                                                   \
template<typename _T> struct has_type_##_NAME_                                                     \
{                                                                                                     \
private:                                                                                              \
	typedef std::true_type yes;                                                                       \
	typedef std::false_type no;                                                                       \
                                                                                                      \
	template<typename U> static auto test(int) ->typename U::_NAME_;                                  \
	template<typename > static no test(...);                                                          \
                                                                                                      \
public:                                                                                               \
                                                                                                      \
	static constexpr bool value = !std::is_same<decltype(test< _T>(0)), no>::value;                   \
}                                                                                                     \
;

#define DECL_RET_TYPE(_EXPR_) ->decltype((_EXPR_)){return (_EXPR_);}

#define ENABLE_IF_DECL_RET_TYPE(_COND_,_EXPR_) \
        ->typename std::enable_if<_COND_,decltype((_EXPR_))>::type {return (_EXPR_);}

namespace _impl
{

//HAS_OPERATOR(sub_script, []);
HAS_MEMBER_FUNCTION(at);

}  // namespace _impl

template<typename _T, typename _Args>
struct is_indexable
{
private:
	typedef std::true_type yes;
	typedef std::false_type no;

	template<typename _U>
	static auto test(int) ->
	decltype(std::declval<_U>() [ std::declval<_Args>() ] );

	template<typename > static no test(...);

public:

	static constexpr bool value =
			(!std::is_same<decltype(test<_T>(0)), no>::value)
					|| ((std::is_array<_T>::value)
							&& (std::is_integral<_Args>::value));

};

template<typename _T>
struct is_shared_ptr
{
	static constexpr bool value = false;
};
template<typename T>
struct is_shared_ptr<std::shared_ptr<T>>
{
	static constexpr bool value = true;
};
template<typename T>
struct is_shared_ptr<const std::shared_ptr<T>>
{
	static constexpr bool value = true;
};

template<typename T, typename TI>
auto get_value(T & v, TI const& s)
ENABLE_IF_DECL_RET_TYPE(
		! (is_indexable<T,TI>::value || is_shared_ptr<T >::value ) , (v))

template<typename T, typename TI>
auto get_value(T & v, TI const & s)
ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TI>::value ), (v[s]))

template<typename T, typename TI>
auto get_value(T & v, TI const& s)
ENABLE_IF_DECL_RET_TYPE(( is_shared_ptr<T >::value ) , v.get()[s])

namespace _impl
{

template<size_t N>
struct recursive_get_value_aux
{
	template<typename T, typename TI>
	static auto eval(T & v, TI const *s)
	DECL_RET_TYPE(
			( recursive_get_value_aux<N-1>::eval(v[s[0]], s+1))
	)
};
template<>
struct recursive_get_value_aux<0>
{
	template<typename T, typename TI>
	static auto eval(T & v, TI const *s)
	DECL_RET_TYPE( ( v ) )
};
}

template<typename T>
struct rank
{
	static constexpr size_t value = std::rank<T>::value;
};
// namespace _impl

template<typename _Tp, _Tp ... _Idx> struct integer_sequence;
template<typename, size_t...> struct nTuple;

template<typename T, typename TI>
auto get_value_r(T & v, TI const *s)
ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TI>::value),
		( _impl::recursive_get_value_aux<rank<T>::value>::eval(v,s)))

template<typename T, typename TI>
auto get_value_r(T & v, TI const * s)
ENABLE_IF_DECL_RET_TYPE((!is_indexable<T,TI>::value), (v))

template<typename T, typename TI, size_t N>
auto get_value_r(T & v, nTuple<TI, N> const &s)
ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TI>::value),
		( _impl::recursive_get_value_aux<N>::eval(v,s)))

template<typename T, typename TI, size_t N>
auto get_value_r(T & v, nTuple<TI, N> const &s)
ENABLE_IF_DECL_RET_TYPE((!is_indexable<T,TI>::value), (v))

template<typename T, typename TI, TI M, TI ...N>
auto get_value(T & v, integer_sequence<TI, M, N...>)
ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TI>::value),
		get_value(v[M],integer_sequence<TI, N...>()))

template<typename T, typename TI, TI M, TI ...N>
auto get_value(T & v, integer_sequence<TI, M, N...>)
ENABLE_IF_DECL_RET_TYPE((!is_indexable<T,TI>::value), v)

//template<typename T, typename TI, TI ...N>
//auto get_value(T & v, integer_sequence<TI, N...>)
//ENABLE_IF_DECL_RET_TYPE((!is_indexable<T,TI>::value), (v))

//template<typename T, typename ...Args>
//auto get_value(std::shared_ptr<T> & v, Args &&... args)
//DECL_RET_TYPE( get_value(v.get(),std::forward<Args>(args)...))
//
//template<typename T, typename ...Args>
//auto get_value(std::shared_ptr<T> const & v, Args &&... args)
//DECL_RET_TYPE( get_value(v.get(),std::forward<Args>(args)...))

/// \note  http://stackoverflow.com/questions/3913503/metaprogram-for-bit-counting
template<unsigned int N> struct CountBits
{
	static const unsigned int n = CountBits<N / 2>::n + 1;
};

template<> struct CountBits<0>
{
	static const unsigned int n = 0;
};

inline unsigned int count_bits(unsigned long s)
{
	unsigned int n = 0;
	while (s != 0)
	{
		++n;
		s = s >> 1;
	}
	return n;
}

template<typename T> inline T* PointerTo(T & v)
{
	return &v;
}

template<typename T> inline T* PointerTo(T * v)
{
	return v;
}

template<typename TV, typename TR> inline TV TypeCast(TR const& obj)
{
	return std::move(static_cast<TV>(obj));
}

template<int...> class int_tuple_t;
//namespace _impl
//{
////******************************************************************************************************
//// Third-part code begin
//// ref: https://gitorious.org/redistd/redistd
//// Copyright Jonathan Wakely 2012
//// Distributed under the Boost Software License, Version 1.0.
//// (See accompanying file LICENSE_1_0.txt or copy at
//// http://www.boost.org/LICENSE_1_0.txt)
//
///// A type that represents a parameter pack of zero or more integers.
//template<unsigned ... Indices>
//struct index_tuple
//{
//	/// Generate an index_tuple with an additional element.
//	template<unsigned N>
//	using append = index_tuple<Indices..., N>;
//};
//
///// Unary metafunction that generates an index_tuple containing [0, Size)
//template<unsigned Size>
//struct make_index_tuple
//{
//	typedef typename make_index_tuple<Size - 1>::type::template append<Size - 1> type;
//};
//
//// Terminal case of the recursive metafunction.
//template<>
//struct make_index_tuple<0u>
//{
//	typedef index_tuple<> type;
//};
//
//template<typename ... Types>
//using to_index_tuple = typename make_index_tuple<sizeof...(Types)>::type;
//// Third-part code end
////******************************************************************************************************
//
//}// namespace _impl

HAS_MEMBER_FUNCTION(begin)
HAS_MEMBER_FUNCTION(end)

template<typename T>
auto begin(T& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_begin<T>::value),( l.begin()))

template<typename T>
auto begin(T& l)
ENABLE_IF_DECL_RET_TYPE((!has_member_function_begin<T>::value),(std::get<0>(l)))

template<typename T>
auto begin(T const& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_begin<T>::value),( l.begin()))

template<typename T>
auto begin(T const& l)
ENABLE_IF_DECL_RET_TYPE((!has_member_function_begin<T>::value),(std::get<0>(l)))

template<typename T>
auto end(T& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_end<T>::value),( l.end()))

template<typename T>
auto end(T& l)
ENABLE_IF_DECL_RET_TYPE((!has_member_function_end<T>::value),(std::get<1>(l)))

template<typename T>
auto end(T const& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_end<T>::value),( l.end()))

template<typename T>
auto end(T const& l)
ENABLE_IF_DECL_RET_TYPE((!has_member_function_end<T>::value),(std::get<1>(l)))

HAS_MEMBER_FUNCTION(rbegin)
HAS_MEMBER_FUNCTION(rend)

template<typename T>
auto rbegin(T& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_begin<T>::value),( l.rbegin()))

template<typename T>
auto rbegin(T& l)
ENABLE_IF_DECL_RET_TYPE(
		(!has_member_function_begin<T>::value),(--std::get<1>(l)))

template<typename T>
auto rend(T& l)
ENABLE_IF_DECL_RET_TYPE((has_member_function_end<T>::value),( l.rend()))

template<typename T>
auto rend(T& l)
ENABLE_IF_DECL_RET_TYPE((!has_member_function_end<T>::value),(--std::get<0>(l)))

template<typename T> T const &compact(T const &v)
{
	return v;
}

template<typename T> void decompact(T const &v, T * u)
{
	*u = v;
}

HAS_MEMBER_FUNCTION(swap)

template<typename T> typename std::enable_if<has_member_function_swap<T>::value,
		void>::type sp_swap(T& l, T& r)
{
	l.swap(r);
}

template<typename T> typename std::enable_if<
		!has_member_function_swap<T>::value, void>::type sp_swap(T& l, T& r)
{
	std::swap(l, r);
}
}
// namespace simpla
#endif /* SP_TYPE_TRAITS_H_ */
