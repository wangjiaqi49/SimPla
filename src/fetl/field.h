/*
 * field.h
 *
 *  Created on: 2013-7-19
 *      Author: salmon
 */

#ifndef FIELD_H_
#define FIELD_H_

#include <fetl/primitives.h>
#include <vector>

namespace simpla
{

/***
 *
 * @brief Field
 *
 * @ingroup Field Expression
 *
 */

template<typename TG, typename TValue>
struct Field: public TG::mesh_type::template Container<TValue>
{
public:

	typedef TG geometry_type;

	typedef typename geometry_type::mesh_type mesh_type;

	static const int IForm = geometry_type::IForm;

	typedef TValue value_type;

	typedef Field<geometry_type, value_type> this_type;

	static const int NUM_OF_DIMS = mesh_type::NUM_OF_DIMS;

	typedef typename mesh_type::template Container<value_type> base_type;

	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef typename mesh_type::index_type index_type;

	typedef typename geometry_type::template field_value_type<value_type> field_value_type;

	mesh_type const &mesh;

	Field(mesh_type const &pmesh, value_type default_value = value_type()) :
			base_type(
					std::move(
							pmesh.template MakeContainer<value_type>(IForm,
									default_value))), mesh(pmesh)
	{
	}

	Field(this_type const & f) = delete;

	Field(this_type &&rhs) = delete;

	virtual ~Field()
	{
	}

	void swap(this_type & rhs)
	{
		base_type::swap(rhs);
	}

#ifdef DEBUG  // check boundary
	inline value_type & operator[](index_type const &s)
	{

		return base_type::at(s);

	}
	inline value_type const & operator[](index_type const &s) const
	{
		return base_type::at(s);
	}
#endif

	template<typename Fun>
	inline void ForEach(Fun const &fun)
	{
		mesh.ForEach(IForm,

		[this,&fun](index_type s)
		{
			fun((*this)[s]);
		}

		);
	}

	template<typename Fun>
	inline void ForEach(Fun const &fun) const
	{
		mesh.ForEach(IForm,

		[this,&fun](index_type s)
		{
			fun((*this)[s]);
		}

		);
	}

	inline this_type & operator=(this_type const & rhs)
	{
		mesh.ForEach(IForm,

		[this, &rhs](index_type s)
		{
			(*this)[s]=rhs[s];
		}

		);
		return (*this);
	}

#define DECL_SELF_ASSIGN( _OP_ )                                                  \
	template<typename TR> inline this_type &                                      \
	operator _OP_(Field<geometry_type, TR> const & rhs)                               \
	{                                                                             \
		mesh.ForEach(IForm,												      \
		[this, &rhs](index_type const &s)                 \
		{	(*this)[s] _OP_ rhs[s];});                                            \
		return (*this);                                                           \
	}                                                                             \
	template<typename TR> inline this_type &                                      \
	operator _OP_(TR const & rhs)                                                 \
	{                                                                             \
		mesh.ForEach(IForm,									                  \
		[this, &rhs](index_type const &s)                 \
		{	(*this)[s] _OP_ rhs ;});                                              \
		return (*this);                                                           \
	}

	DECL_SELF_ASSIGN(=)

DECL_SELF_ASSIGN	(+=)

	DECL_SELF_ASSIGN(-=)

	DECL_SELF_ASSIGN(*=)

	DECL_SELF_ASSIGN(/=)
#undef DECL_SELF_ASSIGN

	inline field_value_type operator()( coordinates_type const &x) const
	{
		return std::move(Gather(x));
	}

	inline field_value_type Gather( coordinates_type const &x) const
	{

		coordinates_type pcoords;

		index_type s = mesh.SearchCell(x, &pcoords);

		return std::move(Gather(s, pcoords));

	}

	inline field_value_type Gather( index_type const & s,coordinates_type const &pcoords) const
	{

		std::vector<index_type> points;

		std::vector<typename geometry_type::gather_weight_type> weights;

		mesh.GetAffectedPoints(Int2Type<IForm>(), s, points);

		mesh.CalcuateWeights(Int2Type<IForm>(), pcoords, weights);

		field_value_type res;

		res *= 0;

		for (auto it1=points.begin(), it2=weights.begin();
				it1!=points.end()&& it2!=weights.end();++it1,++it2 )
		{
			res += this->operator[](*it1) * (*it2);
		}

		return std::move(res);

	}

	template<typename TV >
	inline void Scatter(TV const & v, coordinates_type const &x )
	{
		coordinates_type pcoords;

		index_type s = mesh.SearchCell(x, &pcoords);

		Scatter(v,s,pcoords);

	}
	template<typename TV >
	inline void Scatter(TV const & v,index_type const & s,coordinates_type const &pcoords )
	{

		std::vector<index_type> points;

		std::vector<typename geometry_type::scatter_weight_type> weights;

		mesh.GetAffectedPoints(Int2Type<IForm>(), s, points);

		mesh.CalcuateWeights(Int2Type<IForm>(), pcoords, weights);

		std::vector<value_type> cache(weights.size());

		for (auto it1=cache.begin(), it2=weights.begin();
				it1!=cache.end() && it2!=weights.end();++it1,++it2 )
		{
			// FIXME: this incorrect for vector field interpolation
			*it1 += Dot(v ,*it2);
		}

		Scatter(points,cache);
	}

	inline void Scatter(std::vector<index_type> const & points,std::vector<value_type> & cache)
	{
		//FIXME: this is not thread safe, need a mutex lock

		for (auto it1=points.begin(), it2=cache.begin();
				it1!=points.end()&&it2!=cache.end();++it1,++it2 )
		{
			(*this)[*it1] += *it2;
		}

	}
};

//template<typename TM>
//struct FieldIterator
//{
//	typedef typename TM::index_type index_type;
//
//	index_type idx;
//	bool
//};
}
// namespace simpla

#endif /* FIELD_H_ */
