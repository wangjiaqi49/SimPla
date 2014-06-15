/*
 * parse_config.h
 *
 *  Created on: 2014年6月12日
 *      Author: salmon
 */

#ifndef PARSE_CONFIG_H_
#define PARSE_CONFIG_H_

#include <memory>
#include <functional>
#include <list>
#include <string>

#include "select.h"
#include "../utilities/log.h"
#include "../utilities/sp_type_traits.h"
#include "../fetl/fetl.h"
#include "model.h"
namespace simpla
{

template<typename TF, typename TM, typename TDict>
std::function<void()> CreateCommand(TF * f, Model<TM> const & model, TDict const & dict)
{

	typedef typename TM::coordinates_type coordinates_type;

	typename TM::range_type def_domain;

	if (dict["DefineDomain"])
	{
		try
		{
			def_domain = model.mesh.Select(TF::IForm,
			        dict["DefineDomain"].template as<std::pair<coordinates_type, coordinates_type>>());
		} catch (...)
		{
			PARSER_ERROR("Configure define domain of a command error!");
		}
	}
	else
	{
		def_domain = model.mesh.Select(TF::IForm);
	}

	if (dict["Select"])
	{
		return CreateCommand(f, Filter(def_domain, f->mesh, dict["Select"]), dict["Operation"]);
	}
	else
	{
		return CreateCommand(f, def_domain, dict["Operation"]);
	}
}

template<typename TF, typename TR, typename TDict>
std::function<void()> CreateCommand(TF* f, TR const &def_domain, TDict const & dict)
{
	if (!dict)
	{
		PARSER_ERROR("'Operation' is not defined!");
	}

	typedef typename TF::mesh_type mesh_type;

	typedef typename TF::field_value_type field_value_type;

	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef std::function<field_value_type(Real, coordinates_type const &, field_value_type const &)> field_fun;

	auto op_ = dict.template as<field_fun>();

	std::function<void()> res = [f,def_domain,op_]()
	{
		for(auto s:def_domain)
		{
			auto x = f->mesh.GetCoordinates(s);

			(*f)[s] = f->mesh.Sample(Int2Type<TF::IForm>(), s, op_(f->mesh.GetTime(), x, (*f)(x)));
		}
	};

	return res;

}
}  // namespace simpla

#endif /* PARSE_CONFIG_H_ */