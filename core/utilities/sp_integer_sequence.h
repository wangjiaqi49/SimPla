/*
 * sp_integer_sequence.h
 *
 *  Created on: 2014年9月26日
 *      Author: salmon
 */

#ifndef SP_INTEGER_SEQUENCE_H_
#define SP_INTEGER_SEQUENCE_H_

#include <stddef.h>
#include <tuple>
#include "sp_type_traits.h"

namespace simpla
{

template<typename _Tp, _Tp ... _Idx>
struct integer_sequence
{
private:
	static constexpr size_t size_ = (sizeof...(_Idx));
public:
	typedef _Tp value_type;

	static constexpr size_t size() noexcept
	{
		return size_;
	}

};

template<size_t N, size_t ...S>
struct _make_index_sequence: _make_index_sequence<N - 1, N - 1, S...>
{
};

template<size_t ...S>
struct _make_index_sequence<0, S...>
{
	typedef integer_sequence<size_t, S...> type;
};

template<size_t ... Ints>
using index_sequence = integer_sequence< size_t, Ints...>;

template<size_t N>
using make_index_sequence =typename _make_index_sequence< N>::type;

template<class ... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
//
//template<typename T, typename TInts, TInts M, TInts ...N>
//auto get_value(T &v, integer_sequence<TInts, M, N...>)
//ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TInts>::value),(get_value(v[M],N... )))
//
//template<typename T, typename TInts, TInts M>
//auto get_value(T const &v, integer_sequence<TInts, M>)
//ENABLE_IF_DECL_RET_TYPE((is_indexable<T,TInts>::value),(get_value(v[M] )))

//
//template<typename T, typename TInts, TInts ...N, typename ...Others>
//auto get_value(T const&v, integer_sequence<TInts, N...>, Others && ...others)
//DECL_RET_TYPE((get_value(v,N...,std::forward<Others>(others)...)))

template<size_t ... N> using index_sequence = integer_sequence<size_t , N...>;

template<size_t N, typename ...> struct seq_get;

template<size_t N, typename Tp, Tp M, Tp ...I>
struct seq_get<N, integer_sequence<Tp, M, I ...> >
{
	static constexpr Tp value =
			seq_get<N - 1, integer_sequence<Tp, I ...> >::value;
};

template<typename Tp, Tp M, Tp ...I>
struct seq_get<0, integer_sequence<Tp, M, I ...> >
{
	static constexpr Tp value = M;
};

template<typename Tp>
struct seq_get<0, integer_sequence<Tp> >
{
	static constexpr Tp value = 0;
};

template<typename ...> class cat_integer_sequence;

template<typename T, T ... N>
struct cat_integer_sequence<integer_sequence<T, N ...> >
{
	typedef integer_sequence<T, N ...> type;
};
template<typename T, T ... N>
struct cat_integer_sequence<integer_sequence<T, N ...>, integer_sequence<T>>
{
	typedef integer_sequence<T, N ...> type;
};

template<typename T, T ... N1, T ... N2, typename ...Others>
struct cat_integer_sequence<integer_sequence<T, N1...>,
		integer_sequence<T, N2...>, Others ...>
{
	typedef typename cat_integer_sequence<integer_sequence<T, N1..., N2...>,
			Others...>::type type;
};

template<typename ...> class longer_integer_sequence;

template<typename T, T ... N>
struct longer_integer_sequence<integer_sequence<T, N ...> >
{
	typedef integer_sequence<T, N ...> type;
};
template<typename T, T ... N>
struct longer_integer_sequence<integer_sequence<T, N ...>, integer_sequence<T>>
{
	typedef integer_sequence<T, N ...> type;
};

template<typename T, T ... N1, T ... N2, typename ...Others>
struct longer_integer_sequence<integer_sequence<T, N1...>,
		integer_sequence<T, N2...>, Others ...>
{
	typedef typename std::conditional<(sizeof...(N1) > sizeof...(N2)),
			typename longer_integer_sequence<integer_sequence<T, N1...>,
					Others...>::type,
			typename longer_integer_sequence<integer_sequence<T, N2...>,
					Others...>::type>::type type;

};

//TODO need implement max_integer_sequence, min_integer_sequence

template<size_t...> struct _seq_for;

template<size_t M>
struct _seq_for<M>
{

	template<typename TOP, typename ...Args>
	static inline void eval(TOP const & op, Args && ... args)
	{
		op(get_value(std::forward<Args>(args), M - 1)...);
		_seq_for<M - 1>::eval(op, std::forward<Args>(args)...);
	}

};
template<>
struct _seq_for<0>
{

	template<typename TOP, typename ...Args>
	static inline void eval(TOP const & op, Args && ... args)
	{
	}

};

template<size_t M, size_t ...N>
struct _seq_for<M, N...>
{

	template<typename TOP, typename ...Args>
	static inline void eval(TOP const & op, Args && ... args)
	{
		eval(op, integer_sequence<size_t>(), std::forward<Args>(args)...);
	}

	template<typename TOP, size_t ...L, typename ...Args>
	static inline void eval(TOP const & op, integer_sequence<size_t, L...>,
			Args && ... args)
	{
		_seq_for<N...>::eval(op, integer_sequence<size_t, L..., M>(),
				std::forward<Args>(args)...);

		_seq_for<M - 1, N...>::eval(op, integer_sequence<size_t, L...>(),
				std::forward<Args>(args)...);
	}

};

//template<size_t ...N>
//struct _seq_for<0, N...>
//{
//	template<typename ...Args>
//	static inline void eval(Args && ... args)
//	{
//	}
//};
//
//template<>
//struct _seq_for<>
//{
//
//	template<typename TOP, typename TInts, TInts ...L, typename ...Args>
//	static inline void eval(TOP const & op, integer_sequence<TInts, L...>,
//			Args && ... args)
//	{
//		typedef integer_sequence<size_t, (L-1)...> i_seq;
//
//		op(get_value(std::forward<Args>(args),i_seq()) ...);
//
//	}
//};

template<size_t N, typename ...Args>
void seq_for(integer_sequence<size_t, N>, Args && ... args)
{
	_seq_for<N>::eval(std::forward<Args>(args) ...);
}

template<size_t ... N, typename ...Args>
void seq_for(integer_sequence<size_t, N...>, Args && ... args)
{
	_seq_for<N...>::eval(std::forward<Args>(args) ...);
}

template<size_t...> struct _seq_reduce;

template<size_t M, size_t ...N>
struct _seq_reduce<M, N...>
{

	template<typename Reduction, size_t ...L, typename ... Args>
	static inline auto eval(Reduction const & reduction,
			integer_sequence<size_t, L...>,
			Args &&... args)
					DECL_RET_TYPE(
							(
									reduction( _seq_reduce< N... >::eval( reduction,
													integer_sequence<size_t , L..., M>(),
													std::forward<Args >(args)... ),

											_seq_reduce< M - 1, N... >::eval( reduction,
													integer_sequence<size_t , L...>(),
													std::forward<Args > (args)... ) )
							))

	template<typename Reduction, typename ...Args>
	static inline auto eval(Reduction const & reduction,
			Args && ... args)
					DECL_RET_TYPE(
							( eval( reduction,integer_sequence<size_t >(), std::forward<Args > (args)...) ))

};

template<size_t ...N>
struct _seq_reduce<1, N...>
{

	template<typename Reduction, size_t ...L, typename ...Args>
	static inline auto eval(Reduction const & reduction,
			integer_sequence<size_t, L...>,
			Args &&... args)
					DECL_RET_TYPE(
							(
									_seq_reduce< N... >::eval(
											reduction, integer_sequence<size_t , L..., 1>(),
											std::forward<Args >(args)... )
							)
					)

};

template<>
struct _seq_reduce<>
{
	template<typename Reduction, size_t ...L, typename Args>
	static inline auto eval(Reduction const &, integer_sequence<size_t, L...>,
			Args const& args)
					DECL_RET_TYPE( (get_value( (args),integer_sequence<size_t, (L-1)...>()) ))

};
template<size_t ... N, typename TOP, typename ...Args>
auto seq_reduce(integer_sequence<size_t, N...>, TOP const & op,
		Args && ... args)
		DECL_RET_TYPE( (_seq_reduce<N...>::eval(op,
								std::forward<Args>(args) ...)))

template<typename TInts, TInts ...N, typename TOP>
void seq_for_each(integer_sequence<TInts, N...>, TOP const & op)
{
	size_t ndims = sizeof...(N);
	TInts dims[] =
	{ N... };
	TInts idx[ndims];

	for (int i = 0; i < ndims; ++i)
	{
		idx[i] = 0;
	}

	while (1)
	{

		op(idx);

		++idx[ndims - 1];
		for (int rank = ndims - 1; rank > 0; --rank)
		{
			if (idx[rank] >= dims[rank])
			{
				idx[rank] = 0;
				++idx[rank - 1];
			}
		}
		if (idx[0] >= dims[0])
			break;
	}

}

template<typename TInts, TInts ...N, typename TOS, typename TA>
TOS& seq_print(integer_sequence<TInts, N...>, TOS & os, TA const &d)
{
	size_t ndims = sizeof...(N);
	TInts dims[] =
	{ N... };
	TInts idx[ndims];

	for (int i = 0; i < ndims; ++i)
	{
		idx[i] = 0;
	}

	while (1)
	{

		os << get_value(d, idx) << ", ";

		++idx[ndims - 1];

		for (int rank = ndims - 1; rank > 0; --rank)
		{
			if (idx[rank] >= dims[rank])
			{
				idx[rank] = 0;
				++(idx[rank - 1]);

				if (rank == ndims - 1)
					os << "\n";
			}
		}
		if (idx[0] >= dims[0])
			break;
	}
	return os;
}

}
// namespace simpla
#endif /* SP_INTEGER_SEQUENCE_H_ */
