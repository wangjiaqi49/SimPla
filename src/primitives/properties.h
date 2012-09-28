/*
 * properties.h
 *
 *  Created on: 2012-3-6
 *      Author: salmon
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_
#include <string>
#include <sstream>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "primitives/ntuple.h"

namespace simpla
{
using boost::property_tree::ptree;

void read_file(std::string const & fname, ptree & pt)
{
	std::cout << fname << std::endl;

	std::string ext = fname.substr(fname.rfind("."));

	if (ext == "xml")
	{
		read_xml(fname, pt);
	}
	else if (ext == "ini")
	{
		read_ini(fname, pt);
	}
	else if (ext == "json")
	{
		read_json(fname, pt);
	}
	else if (ext == "info")
	{
		read_info(fname, pt);
	}
}

void write_file(std::string const & fname, ptree const & pt)
{
	std::string ext = fname.substr(fname.rfind("."));

	if (ext == "xml")
	{
		write_xml(fname, pt);
	}
	else if (ext == "ini")
	{
		write_ini(fname, pt);
	}
	else if (ext == "json")
	{
		write_json(fname, pt);
	}
	else if (ext == "info")
	{
		write_info(fname, pt);
	}
}

template<class Ext, class Int = std::string> struct pt_trans;

template<int N, class T>
struct pt_trans<nTuple<N, T>, std::string>
{
	typedef nTuple<N, T> external_type;
	typedef std::string internal_type;

	external_type get_value(const internal_type &value) const
	{
		std::istringstream is(value);
		nTuple<N, T> tv;
		for (int i = 0; i < N && is; ++i)
		{
			is >> tv[i];
		}
		return tv;
	}

	internal_type put_value(const external_type &value) const
	{
		std::ostringstream os;

		for (int i = 0; i < N; ++i)
		{
			os << " " << value[i];
		}
		return os.str();
	}

};
} // namespace simpla
#endif /* PROPERTIES_H_ */
