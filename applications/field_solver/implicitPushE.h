/*
 * implicitPushE.h
 *
 * \date  2014-4-16
 *      \author  salmon
 */

#ifndef IMPLICITPUSHE_H_
#define IMPLICITPUSHE_H_

#include "../../core/field/field.h"
#include "../../core/utilities/log.h"
#include "../../core/physics/physical_constants.h"

namespace simpla
{

/**
 *   \ingroup FieldSolver
 *
 *   \class ImplicitPushE
 *   \brief implicit electric field pusher
 *   \see \ref FDTD_Plasma
 *
 */
template<typename TM>
class ImplicitPushE
{
public:
	typedef TM mesh_type;
	typedef typename mesh_type::scalar_type scalar_type;

	mesh_type const &mesh;

	template<typename TV, size_t iform> using field=Field< TV,Domain<TM,iform> >;

	field<nTuple<scalar_type, 3>, VERTEX> Ev, Bv;

	field<Real, VERTEX> BB;

	typedef field<scalar_type, VERTEX> rho_type;

	typedef field<nTuple<scalar_type, 3>, VERTEX> J_type;

	template<typename ...Others>
	ImplicitPushE(mesh_type const & m, Others const &...) :
			mesh(m), BB(mesh), Ev(mesh), Bv(mesh)
	{
	}

	template<typename TP>
	void next_timestep(field<scalar_type, EDGE> *pdE);
};

/**
 *
 * @param E
 * @param B
 * @param particles
 * @param pdE
 */
template<typename TM>
template<typename TP>
void ImplicitPushE<TM>::next_timestep(field<scalar_type, EDGE> *pdE)
{
//	{
//		bool flag = false;
//		for (auto &p : particles)
//		{
//			flag |= (p.second->is_implicit());
//		}
//
//		if (!flag)
//			return;
//	}
//
//	DEFINE_PHYSICAL_CONST
//
//	Real dt = mesh.get_dt();
//
//	LOGGER << "Implicit Push E ";
//
//	if (Ev.empty())
//		Ev = MapTo<VERTEX>(E1);
//
//	Bv = MapTo<VERTEX>(B1);
//
//	BB = Dot(B0, B0);
//
//	auto Q = mesh.template make_field<VERTEX, nTuple<scalar_type,3>>();
//	auto K = mesh.template make_field<VERTEX, nTuple<scalar_type,3>>();
//
//	Q = MapTo<VERTEX>(*pdE);
//
//	auto a = mesh.template make_field<VERTEX, scalar_type>();
//	auto b = mesh.template make_field<VERTEX, scalar_type>();
//	auto c = mesh.template make_field<VERTEX, scalar_type>();
//
//	a.clear();
//	b.clear();
//	c.clear();
//
//	for (auto &p : particles)
//	{
//		if (p.second->is_implicit())
//		{
//			auto & rhos = (p.second->template rho<rho_type>());
//			auto & Js = p.second->template J<J_type>();
//
//			Real ms = p.second->get_mass();
//			Real qs = p.second->get_charge();
//
//			Real as = (dt * qs) / (2.0 * ms);
//
//			K = (Ev * rhos * 2.0 + Cross(Js, B0)) * as + Js;
//
//			Q -= 0.5 * dt / epsilon0
//			        * ((K + Cross(K, B0) * as + B0 * (Dot(K, B0) * as * as)) / (BB * as * as + 1) + Js);
//
//			a += rhos * as / (BB * as * as + 1);
//			b += rhos * as * as / (BB * as * as + 1);
//			c += rhos * as * as * as / (BB * as * as + 1);
//		}
//	}
//
//	a *= 0.5 * dt / epsilon0;
//	b *= 0.5 * dt / epsilon0;
//	c *= 0.5 * dt / epsilon0;
//	a += 1;
//
//	auto dEv = mesh.template make_field<VERTEX, nTuple<scalar_type,3>>();
//
//	dEv = (Q * a - Cross(Q, B0) * b + B0 * (Dot(Q, B0) * (b * b - c * a) / (a + c * BB))) / (b * b * BB + a * a);
//
//	Ev += dEv;
//
//	*pdE = MapTo<EDGE>(dEv);
//
//	LOGGER << DONE;

}

}
// namespace simpla

#endif /* IMPLICITPUSHE_H_ */
