/*
 * explicit_em_impl.h
 *
 *  Created on: 2013年12月29日
 *      Author: salmon
 */

#ifndef EXPLICIT_EM_IMPL_H_
#define EXPLICIT_EM_IMPL_H_

#include <cmath>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

// Misc
#include "../../src/utilities/log.h"
#include "../../src/utilities/pretty_stream.h"
// Data IO
#include "../../src/io/data_stream.h"

// Field
#include "../../src/fetl/fetl.h"
#include "../../src/fetl/save_field.h"

// Particle
#include "../../src/particle/particle_base.h"

// Model
#include "../../src/model/model.h"
#include "../../src/model/parse_config.h"
#include "../../src/model/geqdsk.h"

// Solver
#include "../field_solver/pml.h"
#include "../field_solver/implicitPushE.h"
#include "../particle_solver/particle_factory.h"

namespace simpla
{

template<typename TM>
struct ExplicitEMContext
{
public:

	typedef TM mesh_type;

	DEFINE_FIELDS(TM)

	typedef ExplicitEMContext<mesh_type> this_type;

	ExplicitEMContext();

	template<typename ...Args>
	ExplicitEMContext(Args const & ...args)
			: ExplicitEMContext()
	{
		Load(std::forward<Args const &>(args)...);
	}
	~ExplicitEMContext();

	template<typename TDict> void Load(TDict const & dict);

	void NextTimeStep();

	std::string Save(std::string const & path = "") const;

	double CheckCourantDt() const;

public:

	mesh_type mesh;

	std::string description;

	Model<mesh_type> model_;

	Form<EDGE> E, dE;
	Form<FACE> B, dB;
	Form<VERTEX> n, n0, phi; // electrostatic potential

	Form<EDGE> J0; //background current density J0+Curl(B(t=0))=0
	Form<EDGE> Jext; // current density

	Field<mesh_type, VERTEX, nTuple<3, Real> > Bv;

	typedef decltype(E) TE;
	typedef decltype(B) TB;
	typedef decltype(Jext) TJ;

	typedef std::map<std::string, std::shared_ptr<ParticleBase<mesh_type> > > TParticles;

	std::function<void(Real, TE const &, TB const &, TE*)> E_plus_CurlB;

	std::function<void(Real, TE const &, TB const &, TB*)> B_minus_CurlE;

	std::function<void(TE const &, TB const &, TParticles const&, TE*)> Implicit_PushE;

	template<typename TBatch>
	void ExcuteCommands(TBatch const & batch)
	{
		for (auto const & command : batch)
		{
			command();
		}
	}

private:

	std::list<std::function<void()> > commandToE_;

	std::list<std::function<void()> > commandToB_;

	std::list<std::function<void()> > commandToJ_;

	std::map<std::string, std::shared_ptr<ParticleBase<mesh_type>>>particles_;

}
;

template<typename TM>
ExplicitEMContext<TM>::ExplicitEMContext()
		: model_(mesh), E(mesh), B(mesh), Jext(mesh), J0(mesh), dE(mesh), dB(mesh), n(mesh), n0(mesh), phi(mesh), Bv(
		        mesh)
{
}

template<typename TM>
ExplicitEMContext<TM>::~ExplicitEMContext()
{
}
template<typename TM> template<typename TDict>
void ExplicitEMContext<TM>::Load(TDict const & dict)
{

	LOGGER << "Load ExplicitEMContext ";

	description = "Description = \"" + dict["Description"].template as<std::string>() + "\"\n";

	LOGGER << description;

	mesh.Load(dict["Grid"]);

	mesh.Decompose();

	Form<VERTEX> ne0(mesh);
	Form<VERTEX> Te0(mesh);
	Form<VERTEX> Ti0(mesh);

	E.Clear();
	B.Clear();
	Jext.Clear();

	dB.Clear();
	dE.Clear();
	J0.Clear();

//	if (dict["Model"])
//	{
//		model_.Update();
//
//		if (dict["Model"]["GFile"])
//		{
//
//			GEqdsk geqdsk(dict["Model"]["GFile"].template as<std::string>());
//
//			nTuple<3, Real> xmin, xmax;
//
//			xmin[0] = geqdsk.GetMin()[0];
//			xmin[1] = geqdsk.GetMin()[1];
//			xmin[2] = 0;
//			xmax[0] = geqdsk.GetMax()[0];
//			xmax[1] = geqdsk.GetMax()[1];
//			xmax[2] = 0;
//
//			mesh.SetExtents(xmin, xmax);
//
//			model_.Add("Plasma", geqdsk.Boundary());
//			model_.Add("Vacuum", geqdsk.Limiter());
//			model_.Update();
//
//			geqdsk.Save(std::cout);
//
//			B.Clear();
//
//			for (auto s : mesh.Select(FACE))
//			{
//				auto x = mesh.CoordinatesToCartesian(mesh.GetCoordinates(s));
//				B[s] = mesh.template Sample<FACE>(Int2Type<FACE>(), s, geqdsk.B(x[0], x[1]));
//
//			}
//
//			ne0.Clear();
//			Te0.Clear();
//			Ti0.Clear();
//
//			for (auto s : model_.Select(VERTEX, "Plasma"))
//			{
//				auto x = mesh.CoordinatesToCartesian(mesh.GetCoordinates(s));
//				auto p = geqdsk.psi(x[0], x[1]);
//
//				ne0[s] = geqdsk.Profile("ne", p);
//				Te0[s] = geqdsk.Profile("Te", p);
//				Ti0[s] = geqdsk.Profile("Ti", p);
//
//			}
//
//			J0 = Curl(B) / CONSTANTS["permeability of free space"];
//
//			description = description + "\n GEqdsk ID:" + geqdsk.Description();
//
//			LOGGER << simpla::Save("ne", ne0);
//			LOGGER << simpla::Save("Te", Te0);
//			LOGGER << simpla::Save("Ti", Ti0);
//		}
//
//	}

	LOG_CMD(LoadField(dict["InitValue"]["E"], &E));

	LOG_CMD(LoadField(dict["InitValue"]["B"], &B));

	LOG_CMD(LoadField(dict["InitValue"]["J"], &Jext));

	LOG_CMD(LoadField(dict["InitValue"]["ne"], &ne0));

	LOG_CMD(LoadField(dict["InitValue"]["Te"], &Te0));

	LOG_CMD(LoadField(dict["InitValue"]["Ti"], &Ti0));

	bool enableImplicit = false;

	bool enablePML = false;

	DEFINE_PHYSICAL_CONST

	if (dict["Particles"])
	{
		LOGGER << "Load Particles";

		for (auto opt : dict["Particles"])
		{

			auto key = opt.first.template as<std::string>("unnamed");

			auto p = ParticleFactory<mesh_type>(mesh, opt.second, ne0, Te0);

			if (p != nullptr)
			{
				particles_.emplace(key, p);

				enableImplicit = enableImplicit || p->EnableImplicit();
			}
		}

	}

	if (dict["Constraints"])
	{
		LOGGER << "Load Constraints";
		for (auto const & item : dict["Constraints"])
		{

			auto dof = item.second["DOF"].template as<std::string>("");

			LOGGER << "Add constraint to " << dof;

			if (dof == "E")
			{
				commandToE_.push_back(CreateCommand(&E, model_, item.second));
			}
			else if (dof == "B")
			{
				commandToB_.push_back(CreateCommand(&B, model_, item.second));
			}
			else if (dof == "J")
			{
				commandToJ_.push_back(CreateCommand(&Jext, model_, item.second));
			}
			else
			{
				UNIMPLEMENT2("Unknown DOF!");
			}
		}
	}

	if (dict["FieldSolver"])
	{
		auto dict_ = dict["FieldSolver"];
		LOGGER << "Load electromagnetic fields solver";

		using namespace std::placeholders;

		Real ic2 = 1.0 / (mu0 * epsilon0);

		if (dict["FieldSolver"]["PML"])
		{
			auto solver = std::shared_ptr<PML<TM> >(new PML<TM>(mesh, dict["FieldSolver"]["PML"]));

			E_plus_CurlB = std::bind(&PML<TM>::NextTimeStepE, solver, _1, _2, _3, _4);

			B_minus_CurlE = std::bind(&PML<TM>::NextTimeStepB, solver, _1, _2, _3, _4);

		}
		else
		{
			E_plus_CurlB = [mu0 , epsilon0](Real dt, TE const & E , TB const & B, TE* pdE)
			{
				auto & dE=*pdE;
				LOG_CMD(dE += Curl(B)/(mu0 * epsilon0) *dt);
			};

			B_minus_CurlE = [](Real dt, TE const & E, TB const &, TB* pdB)
			{
				auto & dB=*pdB;
				LOG_CMD( dB -= Curl(E)*dt);
			};
		}

	}
	Implicit_PushE = [] ( TE const &, TB const &, TParticles const&, TE*)
	{};
	if (enableImplicit)
	{

		auto solver = std::shared_ptr<ImplicitPushE<mesh_type>>(new ImplicitPushE<mesh_type>(mesh));
		Implicit_PushE = [solver] ( TE const & pE, TB const & pB, TParticles const&p, TE*dE)
		{	solver->NextTimeStep( pE,pB,p,dE);};
	}

}

template<typename TM>
std::string ExplicitEMContext<TM>::Save(std::string const & path) const
{
	GLOBAL_DATA_STREAM.OpenGroup(path);

	std::stringstream os;

	os

	<< description

	<< "\n, Grid = { \n" << mesh.Save(path ) << " \n} "
	;

	os

	<< "\n, Fields = {" << "\n"

	<< "\n, E = " << SAVE(E )

	<< "\n, B = " << SAVE(B )

	<< "\n, J = " << SAVE(Jext)

	<< "\n, J0 = " << SAVE(J0 )

	<< "\n} ";

	if (particles_.size() > 0)
	{

		os << "\n , Particles = { \n";
		for (auto const & p : particles_)
		{
			os << p.first << " = { " << p.second->Save(path + "/" + p.first) << "\n},";
		}
		os << "\n} ";
	}

	return os.str();

}

template<typename TM>
void ExplicitEMContext<TM>::NextTimeStep()
{
	Real dt = mesh.GetDt();

	DEFINE_PHYSICAL_CONST

	VERBOSE

	<< "Simulation Time = "

	<< (mesh.GetTime() / CONSTANTS["s"]) << "[s]"

	<< " dt = " << (dt / CONSTANTS["s"]) << "[s]";

	//************************************************************
	// Compute Cycle Begin
	//************************************************************
	// E0 B0,
	LOG_CMD(Jext = J0);
	ExcuteCommands(commandToJ_);

	//   particle 0-> 1/2 . To n[1/2], J[1/2]
	for (auto &p : particles_)
	{
		if (!p.second->EnableImplicit())
		{
			p.second->NextTimeStepZero(E, B);

			auto const & Js = p.second->J();
			LOG_CMD(Jext += Js);
		}
	}

	LOG_CMD(B += dB * 0.5);	//  B(t=0 -> 1/2)
	ExcuteCommands(commandToB_);

	dE.Clear();
	E_plus_CurlB(dt, E, B, &dE); 	// dE += Curl(B)*dt

	LOG_CMD(dE -= Jext * (dt / epsilon0));

	//   particle 1/2 -> 1  . To n[1/2], J[1/2]
	Implicit_PushE(E, B, particles_, &dE);

	LOG_CMD(E += dE);	// E(t=0 -> 1)
	ExcuteCommands(commandToE_);

	dB.Clear();
	B_minus_CurlE(dt, E, B, &dB);

	LOG_CMD(B += dB * 0.5); //	B(t=1/2 -> 1)
	ExcuteCommands(commandToB_);

//************************************************************
// Compute Cycle End
//************************************************************
}

}
// namespace simpla

#endif /* EXPLICIT_EM_IMPL_H_ */
