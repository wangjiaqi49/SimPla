/*
 * manifold.h
 *
 *  Created on: 2014年9月23日
 *      Author: salmon
 */

#ifndef MANIFOLD_H_
#define MANIFOLD_H_

#include <utility>
#include <vector>
#include "../utilities/sp_type_traits.h"
#include "../utilities/primitives.h"
namespace simpla
{

template<typename > class FiniteDiffMethod;
template<typename > class InterpolatorLinear;
template<typename, size_t> class Domain;
template<typename ...> class _Field;
template<typename ...> class Expression;

template<typename TG, //
		template<typename > class Policy1 = FiniteDiffMethod, //
		template<typename > class Policy2 = InterpolatorLinear>
class Manifold: public TG, public Policy1<TG>, public Policy2<TG>
{
public:

	typedef Manifold<TG, Policy1, Policy2> this_type;
	typedef TG geometry_type;
	typedef Policy1<geometry_type> policy1;
	typedef Policy2<geometry_type> policy2;
	typedef typename geometry_type::topology_type topology_type;
	typedef typename geometry_type::coordinates_type coordinates_type;
	typedef typename geometry_type::index_type index_type;
	typedef typename geometry_type::compact_index_type compact_index_type;
	typedef typename geometry_type::iterator iterator;

	template<typename ...Args>
	Manifold(Args && ... args) :
			geometry_type(std::forward<Args>(args)...)
	{
		policy1::geometry(this);
		policy2::geometry(this);
	}

	~Manifold() = default;

	Manifold(this_type const & r) :
			geometry_type((r)), //
			policy1((r)), //
			policy2((r))
	{
	}
	this_type & operator=(this_type const &) = delete;

//	template<typename T, typename TI>
//	auto calculate(T const & expr, TI s) const
//	DECL_RET_TYPE((Policy1<TG>::calculate(expr,s)))

	using Policy1<TG>::calculate;
//	template<typename TC, typename TD>
//	auto calculate(_Field<TC, TD> const & expr,
//			compact_index_type const & s) const
//			DECL_RET_TYPE((expr[s]))
//
//	template<typename TOP, typename TL>
//	auto calculate(_Field<Expression<TOP, TL>> const & expr,
//			compact_index_type const & s) const
//			DECL_RET_TYPE((calcluate(expr.op_, expr.lhs, s)))
//
//	template<typename TOP, typename TL, typename TR>
//	auto get_value(_Field<Expression<TOP, TL, TR>> const & expr,
//			compact_index_type const & s) const
//			DECL_RET_TYPE((calcluate(expr.op_, expr.lhs, expr.rhs, s)))

};

}
// namespace simpla

#endif /* MANIFOLD_H_ */
