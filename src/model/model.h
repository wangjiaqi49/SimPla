/*
 * model.h
 *
 *  Created on: 2013年12月15日
 *      Author: salmon
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <algorithm>
#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../utilities/primitives.h"
#include "../utilities/log.h"
#include "../utilities/pretty_stream.h"
#include "../utilities/utilities.h"
#include "../utilities/range.h"

#include "pointinpolygen.h"
#include "select.h"
namespace std
{
template<typename TI> struct iterator_traits;
}
namespace simpla
{

template<typename TM>
class Model
{

public:
	static constexpr int MAX_NUM_OF_MEIDA_TYPE = 64;
	typedef TM mesh_type;

	typedef std::bitset<MAX_NUM_OF_MEIDA_TYPE> material_type;
	typedef typename mesh_type::iterator iterator;
	typedef typename mesh_type::coordinates_type coordinates_type;

	const material_type null_material;

	std::vector<material_type> material_[mesh_type::NUM_OF_COMPONENT_TYPE];
	std::map<std::string, material_type> register_material_;
	unsigned int max_material_;
	bool isChanged_;
public:

	enum
	{
		NONE = 0, VACUUM = 1, PLASMA, CORE, BOUNDARY, LIMTER,
		// @NOTE: add materials for different physical area or media
		CUSTOM = 20
	};

	mesh_type const &mesh;

	Model(mesh_type const & m)
			: null_material(1 << NONE), mesh(m), max_material_(CUSTOM + 1), isChanged_(true)
	{
		register_material_.emplace("NONE", null_material);

		register_material_.emplace("Vacuum", material_type(1 << VACUUM));
		register_material_.emplace("Plasma", material_type(1 << PLASMA));
		register_material_.emplace("Core", material_type(1 << CORE));
		register_material_.emplace("Boundary", material_type(1 << BOUNDARY));
		register_material_.emplace("Limter", material_type(1 << LIMTER));

	}
	~Model()
	{
	}

	bool empty() const
	{
		return material_[VERTEX].empty();
	}

	operator bool() const
	{
		return material_[VERTEX].empty();
	}

	material_type RegisterMaterial(std::string const & name)
	{
		material_type res;
		if (register_material_.find(name) != register_material_.end())
		{
			res = register_material_[name];
		}
		else if (max_material_ < MAX_NUM_OF_MEIDA_TYPE)
		{
			res.set(max_material_);
			++max_material_;
		}
		else
		{
			RUNTIME_ERROR("Too much media Type");
		}
		return res;
	}

	unsigned int GetNumMaterialType() const
	{
		return max_material_;
	}
	material_type GetMaterialFromNumber(unsigned int material_pos) const
	{
		material_type res;
		res.set(material_pos);
		return std::move(res);
	}
	material_type GetMaterialFromString(std::string const &name) const
	{

		material_type res;

		try
		{
			res = register_material_.at(name);

		} catch (...)
		{
			RUNTIME_ERROR("Unknown material name : " + name);
		}
		return std::move(res);
	}
	material_type GetMaterialFromString(std::string const &name)
	{
		return std::move(RegisterMaterial(name));
	}

	std::vector<material_type> & operator[](unsigned int n)
	{

		return material_[n];
//		auto it = register_material_.find(name);
//		if (it != register_material_.end())
//		{
//			RegisterMaterial(name);
//		}
//
//		return std::move(register_material_.at(name));
	}

	std::vector<material_type> const& operator[](unsigned int n) const
	{
		return material_[n];
	}

	void ClearAll()
	{
		for (auto &v : material_[0])
		{
			v.reset();
		}

		Update();
	}

	template<typename TDict>
	void Load(TDict const & dict)
	{
		if (dict)
		{
			for (auto const & p : dict)
			{
				Modify(p.second);
			}
		}

	}
	std::string Save(std::string const & path, bool is_verbose = false) const
	{
		UNIMPLEMENT;

		return "UNIMPLEMENT!";
	}
	template<typename OS>
	OS & Print(OS &os) const
	{
		UNIMPLEMENT;
		//		os << "{ \n" << "\t -- register media type\n";
		//
		//		for (auto const& p : register_material_)
		//		{
		//			os << std::setw(10) << p.first << " = 0x" << std::hex << p.second.to_ulong() << std::dec << ", \n";
		//		}
		//
		//		os << " }\n"
		//
		//		;
		return os;
	}
	template<typename TCmd>
	void Modify(TCmd const& cmd)
	{
		std::string op = "";
		std::string type = "";

		cmd["Op"].template as<std::string>(&op);
		cmd["Type"].template as<std::string>(&type);

		if (type == "")
		{
			WARNING << "Illegal input! [ undefine type ]";
			return;
		}

		auto select = cmd["Select"];
		if (select.empty())
		{
			std::vector<coordinates_type> r;

			cmd["Range"].as(&r);

			if (op == "Set")
			{
				Set(type, r);
			}
			else if (op == "Remove")
			{
				Remove(type, r);
			}
			else if (op == "Add")
			{
				Add(type, r);
			}
		}
		else
		{
			if (op == "Set")
			{
				Set(type, select);
			}
			else if (op == "Remove")
			{
				Remove(type, select);
			}
			else if (op == "Add")
			{
				Add(type, select);
			}
		}

		LOGGER << op << " material " << type << DONE;
		isChanged_ = true;
	}

	template<typename ...Args> inline
	void Set(std::string material, Args const & ... args)
	{
		Set(GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Set(unsigned int material, Args const & ... args)
	{
		Set(GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	template<typename ...Args> inline
	void Add(std::string material, Args const & ... args)
	{
		Add(GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Add(unsigned int material, Args const & ... args)
	{
		Add(GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	template<typename ...Args> inline
	void Remove(std::string material, Args const & ... args)
	{
		Remove(GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Remove(unsigned int material, Args const & ... args)
	{
		Remove(GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	/**
	 * Set material on vertics
	 * @param material is  set to 1<<material
	 * @param args args are trans-forward to
	 *      SelectVerticsInRange(<lambda function>,mesh,args)
	 */
	template<typename ...Args>
	void Set(material_type material, Args const & ... args)
	{
		_ForEachVertics([&]( material_type &v)
		{	v=material;},

		std::forward<Args const &>(args)...);
	}

	template<typename ...Args>
	void Add(material_type material, Args const & ... args)
	{

		_ForEachVertics([&]( material_type &v)
		{	v|=material;},

		std::forward<Args const &>(args)...);
	}

	template<typename ...Args>
	void Remove(material_type material, Args const & ... args)
	{
		_ForEachVertics([&]( material_type &v)
		{	v&=~material;}, std::forward<Args const &>(args)...);
	}

	void Init(int I = VERTEX)
	{
		size_t num = mesh.GetNumOfElements(I);

		if (material_[I].size() < num)
		{
			material_[I].resize(num, null_material);
		}
	}
	/**
	 *  Update material on edge ,face and cell, base on material on vertics
	 */
	void Update()
	{

		if (isChanged_)
		{
			Init(VERTEX);
			Init(EDGE);
			_UpdateMaterials<EDGE>();
			Init(FACE);
			_UpdateMaterials<FACE>();
			Init(VOLUME);
			_UpdateMaterials<VOLUME>();
			isChanged_ = false;
		}
	}
	bool IsChanged() const
	{
		return isChanged_;
	}

	template<typename TR> using filter_pred_fun_type=std::function<bool(typename TR::iterator::value_type)>;

	template<typename TR> using filter_range_type=
	Range<FilterIterator<std::function<bool(typename TR::iterator::value_type)> , typename TR::iterator>>;

	template<typename TDict>
	filter_range_type<typename mesh_type::range_type> Select(unsigned int iform, TDict const & dict) const
	{
		return Select(mesh.Select(iform), dict);
	}

	template<typename TR, typename TDict>
	filter_range_type<TR> Select(TR const & range, TDict const & dict) const;
	/**
	 *  Choice elements that most close to and out of the interface,
	 *  No element cross interface.
	 * @param
	 * @param fun
	 * @param in_material
	 * @param out_material
	 * @param flag
	 */

	template<typename TR> filter_range_type<TR>
	Select(TR const &range, material_type in, material_type out) const;

	template<typename TR> filter_range_type<TR>
	Select(TR const &range, material_type) const;

	template<typename TR> auto Select(TR const &range, std::string const & in, std::string const & out) const
	DECL_RET_TYPE( Select(range, GetMaterialFromString(in), GetMaterialFromString(out)) )

	template<typename TR> auto Select(TR const &range, char const in[], char const out[]) const
	DECL_RET_TYPE( Select(range, GetMaterialFromString(in), GetMaterialFromString(out)))

	template<typename TR> auto Select(TR const &range, std::string const & m) const
	DECL_RET_TYPE( Select(range, GetMaterialFromString(m)))

	template<typename TR> auto Select(TR const &range, char const m[]) const
	DECL_RET_TYPE( Select(range, GetMaterialFromString(m)))

private:

	/**
	 * Set material on vertics
	 * @param material is  set to 1<<material
	 * @param args args are trans-forward to
	 *      SelectVerticsInRange(<lambda function>,mesh,args)
	 */
	template<typename ...Args>
	void _ForEachVertics(std::function<void(material_type&)> fun, Args const & ... args)
	{

		isChanged_ = true;

		Init(VERTEX);

		for (auto s : Filter(mesh.Select(VERTEX), mesh, std::forward<Args const&>(args)...))
		{
			fun(material_[VERTEX].at(mesh.Hash(s)));
		}
	}

	template<int IFORM>
	void _UpdateMaterials()
	{
		LOGGER << "Update Material " << IFORM;

		try
		{
			for (auto s : mesh.Select(IFORM))
			{
				iterator v[mesh_type::MAX_NUM_VERTEX_PER_CEL];

				int n = mesh.template GetAdjacentCells(Int2Type<IFORM>(), Int2Type<VERTEX>(), s, v);

				material_type flag = null_material;
				for (int i = 0; i < n; ++i)
				{
					flag |= material_[VERTEX].at(mesh.Hash(v[i]));
				}
				material_[IFORM].at(mesh.Hash(s)) = flag;

			}
		} catch (std::out_of_range const &e)
		{
			RUNTIME_ERROR(" I = " + ToString(IFORM) + e.what());
		}
	}
}
;
template<typename TM>
inline std::ostream & operator<<(std::ostream & os, Model<TM> const &self)
{
	self.Print(os);
	return os;
}

template<typename TM>
template<typename TR, typename TDict>
typename Model<TM>::template filter_range_type<TR> Model<TM>::Select(TR const & range, TDict const & dict) const
{
	filter_range_type<TR> res;

	if (!dict["Type"])
		return res;

	auto type = dict["Type"].template as<std::string>("");

	if (type == "Boundary")
	{
		res = Select(range, dict["Material"].template as<std::string>(), "NONE");

	}
	else if (type == "Interface")
	{
		res = Select(range, dict["In"].template as<std::string>(), dict["Out"].template as<std::string>());
	}
	else if (type == "Element")
	{
		res = Select(range, dict["Material"].template as<std::string>());
	}

	return res;

}
template<typename TM>
template<typename TR>
typename Model<TM>::template filter_range_type<TR> Model<TM>::Select(TR const & range, material_type in,
        material_type out) const
{
	if (IsChanged())
	{
		LOGIC_ERROR("need update!!");
	}

	// Good
	//  +----------#----------+
	//  |          #          |
	//  |    A     #-> B   C  |
	//  |          #          |
	//  +----------#----------+
	//
	//  +--------------------+
	//  |         ^          |
	//  |       B |     C    |
	//  |     ########       |
	//  |     #      #       |
	//  |     #  A   #       |
	//  |     #      #       |
	//  |     ########       |
	//  +--------------------+
	//
	//             +----------+
	//             |      C   |
	//  +----------######     |
	//  |          | A  #     |
	//  |    A     | &  #  B  |
	//  |          | B  #->   |
	//  +----------######     |
	//             |          |
	//             +----------+
	//
	//     	       +----------+
	//       C     |          |
	//  +----------#----+     |
	//  |          # A  |     |
	//  |    B   <-# &  |  A  |
	//  |          # B  |     |
	//  +----------#----+     |
	//             |          |
	//             +----------+
	//
	//
	// 	 Bad
	//
	//  +--------------------+
	//  |                    |
	//  |        A           |
	//  |     ########       |
	//  |     #      #       |
	//  |     #->B C #       |
	//  |     #      #       |
	//  |     ########       |
	//  +--------------------+
	//
	// 	            +----------+
	//              |          |
	//   +-------+  |          |
	//   |       |  |          |
	//   |   B   |  |    A     |
	//   |       |  |          |
	//   +-------+  |          |
	//              |          |
	//              +----------+

	filter_pred_fun_type<TR> pred =

	[=]( typename TR::iterator::value_type s )->bool
	{
		auto iform = mesh.IForm(s);

		if ((this->material_[iform].at(this->mesh.Hash(s)) & in).none()
				&& (this->material_[iform].at(this->mesh.Hash(s)) & out).any())
		{
			iterator neighbours[mesh_type::MAX_NUM_NEIGHBOUR_ELEMENT];

			int num=0;
			switch(iform)
			{	case VERTEX:
				num= this->mesh.GetAdjacentCells(Int2Type<VERTEX>(), Int2Type<VOLUME>(), s, neighbours);
				break;
				case EDGE:
				num= this->mesh.GetAdjacentCells(Int2Type<EDGE>(), Int2Type<VOLUME>(), s, neighbours);
				break;
				case FACE:
				num= this->mesh.GetAdjacentCells(Int2Type<FACE>(), Int2Type<VOLUME>(), s, neighbours);
				break;
				case VOLUME:
				num= this->mesh.GetAdjacentCells(Int2Type<VOLUME>(), Int2Type<VOLUME>(), s, neighbours);
				break;
			}
			for (int i = 0; i < num; ++i)
			{
				if (((this->material_[VOLUME].at(this->mesh.Hash(neighbours[i])) & in)).any())
				{
					return true;
				}
			}
		}

		return false;
	};

	return make_filter_range(pred, range);

}

template<typename TM> template<typename TR>
typename Model<TM>::template filter_range_type<TR> Model<TM>::Select(TR const & range, material_type material) const
{
	filter_pred_fun_type<TR> pred = [= ]( typename TM::iterator::value_type s )->bool
	{
		return (((this->material_[this->mesh.IForm(s )].at(this->mesh.Hash(s )) & material)).any());
	};
	return make_filter_range(pred, range);

}

}
// namespace simpla

#endif /* MODEL_H_ */