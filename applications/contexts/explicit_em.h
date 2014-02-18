/*
 * explicit_em_impl.h
 *
 *  Created on: 2013年12月29日
 *      Author: salmon
 */

#ifndef EXPLICIT_EM_IMPL_H_
#define EXPLICIT_EM_IMPL_H_

#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "../../src/fetl/field.h"
#include "../../src/fetl/ntuple.h"
#include "../../src/fetl/primitives.h"
#include "../../src/fetl/save_field.h"
#include "../../src/io/data_stream.h"
#include "../../src/modeling/constraint.h"
#include "../../src/particle/particle_factory.h"
#include "../../src/mesh/field_convert.h"
#include "../../src/utilities/geqdsk.h"
#include "../../src/utilities/log.h"
#include "../../src/utilities/singleton_holder.h"

namespace simpla
{
template<typename TM>
struct ExplicitEMContext
{
public:

	typedef TM mesh_type;

	DEFINE_FIELDS (TM)

	typedef ExplicitEMContext<mesh_type> this_type;

	ExplicitEMContext();

	~ExplicitEMContext();

	template<typename TDict> void Load(TDict const & dict);

	std::ostream & Save(std::ostream & os) const;

	void NextTimeStep();

	void DumpData(std::string const & path = "") const;

public:

	mesh_type mesh;

	std::string description;

	bool isCompactStored_;

	Form<EDGE> E, dE;
	Form<FACE> B, dB;
	Form<VERTEX> phi; // electrostatic potential

	Form<EDGE> J;     // current density
	Form<VERTEX> rho; // charge density

	typedef decltype(E) TE;
	typedef decltype(B) TB;
	typedef decltype(J) TJ;

	std::function<void(Real, TE const &, TB const &, TE*)> CalculatedE;

	std::function<void(Real, TE const &, TB const &, TB*)> CalculatedB;

	void ApplyConstraintToE(TE* pE)
	{
		for (auto const & foo : constraintToE_)
		{
			foo(pE);
		}
	}
	void ApplyConstraintToB(TB* pB)
	{
		for (auto const & foo : constraintToB_)
		{
			foo(pB);
		}
	}
	void ApplyConstraintToJ(TJ* pJ)
	{
		for (auto const & foo : constraintToJ_)
		{
			foo(pJ);
		}
	}

private:

	std::list<std::function<void(TE*)> > constraintToE_;

	std::list<std::function<void(TB*)> > constraintToB_;

	std::list<std::function<void(TJ*)> > constraintToJ_;

	typedef ParticleWrap<TE, TB, TJ> ParticleType;

	std::map<std::string, ParticleType> particles_;

}
;

template<typename TM>
ExplicitEMContext<TM>::ExplicitEMContext() :
		isCompactStored_(true), E(mesh), B(mesh), J(mesh), dE(mesh), dB(mesh), rho(mesh), phi(mesh)
{

	CalculatedE = [](Real dt, TE const & , TB const & pB, TE* pdE)
	{	LOG_CMD(*pdE += Curl(pB)*dt);};

	CalculatedB = [](Real dt, TE const & pE, TB const &, TB* pdB)
	{	LOG_CMD(*pdB -= Curl(pE)*dt);};

}

template<typename TM>
ExplicitEMContext<TM>::~ExplicitEMContext()
{
}
template<typename TM> template<typename TDict>
void ExplicitEMContext<TM>::Load(TDict const & dict)
{
	description = dict["Description"].template as<std::string>();

	mesh.Load(dict["Grid"]);

	B.Clear();
	J.Clear();
	E.Clear();

	dB.Clear();
	dE.Clear();

	if (dict["GFile"])
	{

		GEqdsk geqdsk(dict["GFile"].template as<std::string>());

		mesh.SetExtent(geqdsk.GetMin(), geqdsk.GetMax());

		mesh.Update();

		RForm<EDGE> B1(mesh);

		B1.Clear();

		mesh.SerialTraversal(EDGE,

		[&](typename mesh_type::index_type s,typename mesh_type::coordinates_type const &x)
		{
			B1[s] = mesh.template GetWeightOnElement<FACE>(geqdsk.B(x),s);
		});

		MapTo(B1, &B);

		mesh.tags().Add(MediaTag<TM>::PLASMA, geqdsk.Boundary());
		mesh.tags().Add(MediaTag<TM>::VACUUM, geqdsk.Limiter());
		mesh.tags().Update();
	}

	LOGGER << "Load initial Field";
	if (dict["InitValue"])
	{
		auto init_value = dict["InitValue"];

		LoadField(init_value["E"], &E);
		LOGGER << "Load E" << DONE;
		LoadField(init_value["B"], &B);
		LOGGER << "Load B" << DONE;
		LoadField(init_value["J"], &J);
		LOGGER << "Load J" << DONE;
	}

	LOGGER << "Load Particles";
	for (auto const &opt : dict["Particles"])
	{
		ParticleWrap<TE, TB, TJ> p;

		if (CreateParticle<Mesh, TE, TB, TJ>(mesh, opt.second, &p))
			particles_.emplace(std::make_pair(opt.first.template as<std::string>(), p));
	}

	LOGGER << "Load Constraints";
	for (auto const & item : dict["Constraints"])
	{
		auto dof = item.second["DOF"].template as<std::string>();
		CHECK(dof);
		if (dof == "E")
		{
			constraintToE_.push_back(Constraint<mesh_type, TE::IForm>::template Create<TE>(mesh, item.second));
		}
		else if (dof == "B")
		{
			constraintToB_.push_back(Constraint<mesh_type, TB::IForm>::template Create<TB>(mesh, item.second));
		}
		else if (dof == "J")
		{
			constraintToJ_.push_back(Constraint<mesh_type, TJ::IForm>::template Create<TJ>(mesh, item.second));
		}
		else
		{
			//TODO Add particles constraints
			UNIMPLEMENT2("Unknown Constraints!!");
			continue;
		}

		LOGGER << "Add constraint to " << dof << DONE;
	}

//	CreateEMSolver(cfg["FieldSolver"], mesh, &CalculatedE, &CalculatedB);

	LOGGER << "We have load every thing!";
}

template<typename TM>
std::ostream & ExplicitEMContext<TM>::Save(std::ostream & os) const
{

	os << "Description=\"" << description << "\" \n";

	os << "Grid = " << mesh << "\n";

//	os << " FieldSolver={ \n";
//
//	if (cold_fluid_ != nullptr)
//		os << *cold_fluid_ << ",\n";
//
//	if (pml_ != nullptr)
//		os << *pml_ << ",\n";
//
//	os << "} \n";
//
//	if (particles_ != nullptr)
//		os << *particles_ << "\n";
//
//	os << "Function={";
//	for (auto const & p : field_boundary_)
//	{
//		os << "\"" << p.first << "\",\n";
//	}
	os << "}\n"

	<< "Fields={" << "\n"

	<< "	E = " << Dump(E, "E", false) << ",\n"

	<< "	B = " << Dump(B, "B", false) << ",\n"

	<< "	J = " << Dump(J, "J", false) << ",\n"

	<< "}" << "\n"

	;
	return os;
}
template<typename TM>
void ExplicitEMContext<TM>::NextTimeStep()
{
	Real dt = mesh.GetDt();

	mesh.CheckCourant(dt);

	mesh.NextTimeStep();

	DEFINE_PHYSICAL_CONST(mesh.constants());

	LOGGER

	<< "Simulation Time = "

	<< (mesh.GetTime() / mesh.constants()["s"]) << "[s]"

	<< " dt = " << (dt / mesh.constants()["s"]) << "[s]";

//************************************************************
// Compute Cycle Begin
//************************************************************

	ApplyConstraintToJ(&J);

	dE.Clear();

	// dE = Curl(B)*dt
	CalculatedE(dt, E, B, &dE);

	LOG_CMD(dE -= J / epsilon0 * dt);

	// E(t=0  -> 1/2  )
	LOG_CMD(E += dE * 0.5);

	ApplyConstraintToE(&E);

	for (auto &p : particles_)
	{
		p.second.NextTimeStep(dt, E, B);	// particle(t=0 -> 1)
	}

	//  E(t=1/2  -> 1)
	LOG_CMD(E += dE * 0.5);

	ApplyConstraintToE(&E);

	Form<2> dB(mesh);

	dB.Clear();

	CalculatedB(dt, E, B, &dB);

	//  B(t=1/2 -> 1)
	LOG_CMD(B += dB * 0.5);

	ApplyConstraintToB(&B);

	J.Clear();

	for (auto &p : particles_)
	{
		// B(t=0) E(t=0) particle(t=0) Jext(t=0)
		p.second.Collect(&J, E, B);
	}

// B(t=0 -> 1/2)
	LOG_CMD(B += dB * 0.5);

	ApplyConstraintToB(&B);

//************************************************************
// Compute Cycle End
//************************************************************

}
template<typename TM>
void ExplicitEMContext<TM>::DumpData(std::string const & path) const
{
	GLOBAL_DATA_STREAM.OpenGroup(path);

	LOGGER << DUMP(E);
	LOGGER << DUMP(B);
	LOGGER << DUMP(J);

}
}
// namespace simpla

#endif /* EXPLICIT_EM_IMPL_H_ */
