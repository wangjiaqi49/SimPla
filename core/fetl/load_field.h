/*
 * load_field.h
 *
 *  created on: 2013-12-9
 *      Author: salmon
 */

#ifndef LOAD_FIELD_H_
#define LOAD_FIELD_H_

#include <string>

#include "../utilities/log.h"
#include "fetl.h"

namespace simpla
{
template<typename, unsigned int, typename > class Field;

template<typename TDict, unsigned int IFORM, typename TM, typename Container>
bool load_field_(TDict const &dict, Field<TM, IFORM, Container> *f)
{
	if (!dict)
		return false;

	typedef TM mesh_type;
	typedef typename Field<TM, IFORM, Container>::value_type value_type;
	typedef typename Field<TM, IFORM, Container>::field_value_type field_value_type;

	mesh_type const &mesh = f->mesh;

	f->clear();

	if (dict.is_function())
	{

		for (auto s : mesh.select(IFORM))
		{
			auto x = mesh.get_coordinates(s);

			auto v = dict(x).template as<field_value_type>();

			(*f)[s] = mesh.Sample(std::integral_constant<unsigned int, IFORM>(), s, v);
		}

	}
	else if (dict.is_number() | dict.is_table())
	{

		auto v = dict.template as<field_value_type>();

		for (auto s : mesh.select(IFORM))
		{
			auto x = mesh.get_coordinates(s);

			(*f)[s] = mesh.Sample(std::integral_constant<unsigned int, IFORM>(), s, v);
		}

	}
	else if (dict.is_string())
	{
		std::string url = dict.template as<std::string>();
		//TODO Read field from data file
		UNIMPLEMENT << "Read field from data file or other URI";

		return false;
	}

	update_ghosts(f);

	return true;
}
template<int DIMS, typename TV, typename TDict, unsigned int IFORM, typename TM, typename Container>
bool load_field_wrap(nTuple<DIMS, std::complex<TV> >, TDict const &dict, Field<TM, IFORM, Container> *f)
{

	auto ff = f->mesh.template make_field<IFORM, nTuple<DIMS, Real>>();

	ff.clear();

	bool success = load_field_(dict, &ff);

	if (success)
		*f = ff;

	return success;
}

template<typename TV, typename TDict, unsigned int IFORM, typename TM, typename Container>
bool load_field_wrap(std::complex<TV>, TDict const &dict, Field<TM, IFORM, Container> *f)
{

	auto ff = f->mesh.template make_field<IFORM, Real>();

	ff.clear();

	bool success = load_field_(dict, &ff);

	if (success)
		*f = ff;

	return success;
}

template<typename TV, typename TDict, unsigned int IFORM, typename TM, typename Container>
bool load_field_wrap(TV, TDict const &dict, Field<TM, IFORM, Container> *f)
{
	return load_field_(dict, f);
}

template<typename TDict, unsigned int IFORM, typename TM, typename Container>
bool load_field(TDict const &dict, Field<TM, IFORM, Container> *f)
{
	typedef typename Field<TM, IFORM, Container>::value_type value_type;

	return load_field_wrap(value_type(), dict, f);
}

//template<typename TDict>
//void AssignFromDict(TDict const & dict)
//{
//	Clear();
//
//	if (dict.is_function())
//	{
//
//		Assign(
//
//		[dict](coordinates_type x)->field_value_type
//		{
//			return dict(x[0],x[1],x[2]).template as<field_value_type>();
//		}
//
//		);
//
//	}
//	else if (dict.is_number() && !is_nTuple<field_value_type>::value)
//	{
//		field_value_type v = dict.template as<field_value_type>();
//
//		Assign([v](coordinates_type )->field_value_type
//		{
//			return v;
//		});
//
//	}
//	else if (dict.is_table() && is_nTuple<field_value_type>::value)
//	{
//		field_value_type v = dict.template as<field_value_type>();
//
//		Assign([v](coordinates_type )->field_value_type
//		{
//			return v;
//		});
//
//	}
//	else
//	{
//		WARNING << "Can not assign field from 'dict'!";
//	}
//}
}// namespace simpla

#endif /* LOAD_FIELD_H_ */