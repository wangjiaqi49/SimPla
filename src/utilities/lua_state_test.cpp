/*
 * lua_parser_test.cpp
 *
 *  Created on: 2013年9月24日
 *      Author: salmon
 */

#include "lua_state.h"
#include "fetl/primitives.h"
#include <iostream>
#include <map>
using namespace simpla;
int main(int argc, char** argv)
{
	LuaObject pt;
	pt.Init();

	if (argc > 1)
	{
		pt.ParseFile(argv[1]);
	}
	else
	{
		pt.ParseString(
				"c=100 \n t1={a=5,b=6.0,c=\"text\",e={a=5,b=6.0}} \n t2={e=4,f=true} \n t3={1,3,4,5}\n"
						"tt={6,6,7,3,e=12, d=13,h=2} \n"
						"function f(x,y) \n"
						"    return x+y  \n"
						"end \n");
	}

	for (int i = 0; i < 10; ++i)
	{
		auto l = pt["f"];

		l(1.0, 2.0).as<int>();
	}

	std::cout << "c \t=" << pt.at("c").as<double>() << std::endl;

	std::cout << "c \t=" << pt["c"].as<int>() << std::endl;

	std::cout << "c \t=" << pt["c"].as<double>() << std::endl;

	std::cout << "b \t=" << pt.Get<int>("b", 120) << std::endl;
////
//	auto tt1 = pt.at("t1");
//
////	std::cout << "t1 \t=" << tt1.as<double>() << std::endl;
//
////	std::cout << "t2.f \t=" << std::boolalpha
////			<< pt.GetChild("t2")["f"].as<bool>() << std::endl;
//
	std::cout << "f(2,2.5) \t=" << pt["f"](2.0, 2.5).as<double>() << std::endl;

	std::cout << "f(1.2,2.5) \t=" << pt["f"](1.2, 2.5).as<double>()
			<< std::endl;

	std::cout << "f(3,2.5) \t=" << pt["f"](3.0, 2.5).as<double>() << std::endl;

	std::cout << "f(3,2.5) \t=" << pt["f"](2.0, 2.5).as<double>() << std::endl;

	for (int i = 0; i < 10; ++i)
	{

		std::cout << "t3 \t=" << pt.GetChild("t3").as<nTuple<3, double>>()
				<< std::endl;
	}

	LuaObject * t1 = new LuaObject(pt["t1"]);

	LuaObject * e = new LuaObject(t1->GetChild("e"));

	delete t1;

	std::cout << "t1.e.a \t= " << e->GetChild("a").as<double>() << std::endl;

	delete e;

	for (auto it = pt.GetChild("tt").begin(), e = pt.GetChild("tt").end();
			it != e; ++it)
	{
		std::cout << (*it).first.as<std::string>() << " = "
				<< (*it).second.as<int>() << std::endl;

	}
	std::cout << "============================" << std::endl;
	auto pp = pt.GetChild("tt");

	for (auto it = pp.begin(), e = pp.end(); it != e; ++it)
	{
		std::cout << (*it).first.as<std::string>() << " = "
				<< (*it).second.as<int>() << std::endl;

	}
	std::cout << "============================" << std::endl;
	pt.ParseString("tt.e=1000");

	for (auto it = pp.begin(), e = pp.end(); it != e; ++it)
	{
		std::cout << (*it).first.as<std::string>() << " = "
				<< (*it).second.as<int>() << std::endl;

	}
	std::cout << "============================" << std::endl;
	//	pt.GetChild("tt").ForEach(
//
//	[&](LuaObject const& key,LuaObject const&value)
//	{
//		std::cout << key.as<std::string>()
//		<<" = "<< value.as<int>() << std::endl;
//	}
//
//	);
//	auto obj = pt.GetChild("tt");
//	size_t num = obj.GetLength();
//	for (size_t i = 0; i < num; ++i)
//	{
//		std::cout << obj[i].as<int>() << std::endl;
//	}
//
	for (auto const &p : pt.GetChild("tt"))
	{
		std::cout << p.first.as<std::string>() << " = " << p.second.as<int>()
				<< std::endl;
	}
	for (auto const &p : pt.GetChild("tt"))
	{
		std::cout << p.first.as<std::string>() << " = " << p.second.as<int>()
				<< std::endl;
	}
	for (auto const &p : pt.GetChild("tt"))
	{
		std::cout << p.first.as<std::string>() << " = " << p.second.as<int>()
				<< std::endl;
	}

}
