/*
 * mesh_rectangle.h
 *
 *  Created on: 2014年2月26日
 *      Author: salmon
 */

#ifndef MESH_RECTANGLE_H_
#define MESH_RECTANGLE_H_

#include <cmath>
#include <iostream>
#include <memory>
#include <type_traits>

#include "../fetl/fetl.h"
#include "../utilities/type_utilites.h"
#include "interpolator.h"
namespace simpla
{

template<typename TGeometry> class Mesh;
class OcForest;

template<template<typename > class TGeometry>
class Mesh<TGeometry<OcForest>> : public TGeometry<OcForest>
{
public:
	typedef Mesh<TGeometry<OcForest>> this_type;

	typedef TGeometry<OcForest> geometry_type;

	typedef typename geometry_type::topology_type topology_type;

	typedef Interpolator<this_type> interpolator_type;

	typedef Real scalar_type;

	static constexpr unsigned int NDIMS = 3;

	static constexpr int NUM_OF_COMPONENT_TYPE = NDIMS + 1;

	typedef typename topology_type::coordinates_type coordinates_type;
	typedef typename topology_type::iterator index_type;
	typedef typename topology_type::compact_index_type compact_index_type;

	Mesh() :
			geometry_type()
	{
	}

	template<typename TDict>
	Mesh(TDict const & dict) :
			geometry_type()
	{
		Load(dict);
	}

	~Mesh()
	{
	}

	Mesh(const this_type&) = delete;

	this_type & operator=(const this_type&) = delete;

	inline bool operator==(this_type const & r) const
	{
		return (this == &r);
	}

//	template<typename TDict, typename ...Others>
//	void Load(TDict const & dict, Others const &...others)
//	{
//		geometry_type::Load(dict, std::forward<Others const&>(others)...);
//	}
//
//	std::string Save(std::string const &path) const
//	{
//		return geometry_type::Save(path);
//	}
//
//	std::string Print() const
//	{
//		return geometry_type::Print();
//	}

//***************************************************************************************************
//*	Miscellaneous
//***************************************************************************************************

//***************************************************************************************************
	Real CheckCourantDt(nTuple<3, Real> const & u) const
	{

		Real dt = geometry_type::GetDt();

		auto dims = geometry_type::GetDimensions();
		auto extent = geometry_type::GetExtent();

		Real r = 0.0;
		for (int s = 0; s < 3; ++s)
		{
			if (dims[s] > 1)
			{
				r += u[s] / (extent.second[s] - extent.first[s]);
			}
		}

		if (dt * r > 1.0)
		{
			dt = 0.5 / r;
		}

		return dt;
	}

	Real CheckCourantDt(Real speed) const
	{
		return CheckCourantDt(nTuple<3, Real>(
		{ speed, speed, speed }));
	}

	template<int IFORM, typename TExpr>
	inline typename Field<this_type, IFORM, TExpr>::field_value_type Gather(Field<this_type, IFORM, TExpr> const &f,
			coordinates_type x) const
	{
		return std::move(interpolator_type::Gather(f, x));

	}

	template<int IFORM, typename TExpr>
	inline void Scatter(coordinates_type x, typename Field<this_type, IFORM, TExpr>::field_value_type const & v,
			Field<this_type, IFORM, TExpr> *f) const
	{
		interpolator_type::Scatter(x, v, f);
	}

//***************************************************************************************************
// Exterior algebra
//***************************************************************************************************

	template<typename TL> inline auto OpEval(Int2Type<EXTRIORDERIVATIVE>, Field<this_type, VERTEX, TL> const & f,
			index_type s) const-> decltype(f[s]-f[s])
	{
		auto d = topology_type::DeltaIndex(s.self_);
		return (f[s + d] - f[s - d]);
	}

	template<typename TL> inline auto OpEval(Int2Type<EXTRIORDERIVATIVE>, Field<this_type, EDGE, TL> const & f,
			index_type s) const-> decltype(f[s]-f[s])
	{
		auto X = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);

		return (f[s + Y] - f[s - Y]) - (f[s + Z] - f[s - Z]);
	}

	template<typename TL> inline auto OpEval(Int2Type<EXTRIORDERIVATIVE>, Field<this_type, FACE, TL> const & f,
			index_type s) const-> decltype(f[s]-f[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return (f[s + X] - f[s - X]) + (f[s + Y] - f[s - Y]) + (f[s + Z] - f[s - Z]);
	}

	template<int IL, typename TL> void OpEval(Int2Type<EXTRIORDERIVATIVE>, Field<this_type, IL, TL> const & f,
			index_type s) const = delete;

	template<int IL, typename TL> void OpEval(Int2Type<CODIFFERENTIAL>, Field<this_type, IL, TL> const & f,
			index_type s) const = delete;

	template<typename TL> inline auto OpEval(Int2Type<CODIFFERENTIAL>, Field<this_type, EDGE, TL> const & f,
			index_type s) const->decltype(f[s]-f[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);
		auto a = geometry_type::InvDualVolume(s) * geometry_type::Volume(s);
		return

		-(

		(

			f[s + X] * (geometry_type::InvVolume(s + X) * geometry_type::DualVolume(s + X))

		- f[s - X] * (geometry_type::InvVolume(s - X) * geometry_type::DualVolume(s - X))

		) + (

		f[s + Y] * (geometry_type::InvVolume(s + Y) * geometry_type::DualVolume(s + Y))

		- f[s - Y] * (geometry_type::InvVolume(s - Y) * geometry_type::DualVolume(s - Y))

		) + (

		f[s + Z] * (geometry_type::InvVolume(s + Z) * geometry_type::DualVolume(s + Z))

		- f[s - Z] * (geometry_type::InvVolume(s - Z) * geometry_type::DualVolume(s - Z))

		)

		) * a;

	}

	template<typename TL> inline auto OpEval(Int2Type<CODIFFERENTIAL>, Field<this_type, FACE, TL> const & f,
			index_type s) const-> decltype(f[s]-f[s])
	{
		auto X = topology_type::DeltaIndex(s.self_);
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);
		Real a = geometry_type::InvDualVolume(s) * geometry_type::Volume(s);
		return (

		f[s + Y] * (geometry_type::InvVolume(s + Y) * geometry_type::DualVolume(s + Y))

		- f[s - Y] * (geometry_type::InvVolume(s - Y) * geometry_type::DualVolume(s - Y))

		- f[s + Z] * (geometry_type::InvVolume(s + Z) * geometry_type::DualVolume(s + Z))

		+ f[s - Z] * (geometry_type::InvVolume(s - Z) * geometry_type::DualVolume(s - Z))

		) * a;
	}

	template<typename TL> inline auto OpEval(Int2Type<CODIFFERENTIAL>, Field<this_type, VOLUME, TL> const & f,
			index_type s) const-> decltype(f[s]-f[s])
	{
		auto d = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto a = geometry_type::InvDualVolume(s) * geometry_type::Volume(s);
		return

		-(

		f[s + d] * (geometry_type::InvVolume(s + d) * geometry_type::DualVolume(s + d))

		- f[s - d] * (geometry_type::InvVolume(s - d) * geometry_type::DualVolume(s - d))

		) * a;
	}

//***************************************************************************************************

//! Form<IR> ^ Form<IR> => Form<IR+IL>
	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, VERTEX, TL> const &l,
			Field<this_type, VERTEX, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		return l[s] * r[s] * geometry_type::InvVolume(s);
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, VERTEX, TL> const &l,
			Field<this_type, EDGE, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(s.self_);

		return (l[s - X] * geometry_type::InvVolume(s - X) + l[s + X] * geometry_type::InvVolume(s + X)) * 0.5 * r[s];
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, VERTEX, TL> const &l,
			Field<this_type, FACE, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);

		return (

		l[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z) +

		l[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z) +

		l[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z) +

		l[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

		) * 0.25 * r[s];
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, VERTEX, TL> const &l,
			Field<this_type, VOLUME, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return (

		l[((s - X) - Y) - Z] * geometry_type::InvVolume(((s - X) - Y) - Z) +

		l[((s - X) - Y) + Z] * geometry_type::InvVolume(((s - X) - Y) + Z) +

		l[((s - X) + Y) - Z] * geometry_type::InvVolume(((s - X) + Y) - Z) +

		l[((s - X) + Y) + Z] * geometry_type::InvVolume(((s - X) + Y) + Z) +

		l[((s + X) - Y) - Z] * geometry_type::InvVolume(((s + X) - Y) - Z) +

		l[((s + X) - Y) + Z] * geometry_type::InvVolume(((s + X) - Y) + Z) +

		l[((s + X) + Y) - Z] * geometry_type::InvVolume(((s + X) + Y) - Z) +

		l[((s + X) + Y) + Z] * geometry_type::InvVolume(((s + X) + Y) + Z)

		) * 0.125 * r[s];
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, EDGE, TL> const &l,
			Field<this_type, VERTEX, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(s.self_);
		return l[s] * (r[s - X] + r[s + X]) * 0.5;
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, EDGE, TL> const &l,
			Field<this_type, EDGE, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto Y = topology_type::DeltaIndex(topology_type::Roate(topology_type::Dual(s.self_)));
		auto Z = topology_type::DeltaIndex(topology_type::InverseRoate(topology_type::Dual(s.self_)));

		return (

		(l[s - Y] * geometry_type::InvVolume(s - Y) +

		l[s + Y] * geometry_type::InvVolume(s + Y)

		) * (l[s - Z] * geometry_type::InvVolume(s - Z) +

		l[s + Z] * geometry_type::InvVolume(s + Z)

		) * 0.25 * geometry_type::Volume(s));
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, EDGE, TL> const &l,
			Field<this_type, FACE, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return

		((

		l[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z) +

		l[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z) +

		l[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z) +

		l[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

		) * (

		r[s - X] * geometry_type::InvVolume(s - X) +

		r[s + X] * geometry_type::InvVolume(s + X)

		) + (

		l[(s - Z) - X] * geometry_type::InvVolume((s - Z) - X) +

		l[(s - Z) + X] * geometry_type::InvVolume((s - Z) + X) +

		l[(s + Z) - X] * geometry_type::InvVolume((s + Z) - X) +

		l[(s + Z) + X] * geometry_type::InvVolume((s + Z) + X)

		) * (

		r[s - Y] * geometry_type::InvVolume(s - Y) +

		r[s + Y] * geometry_type::InvVolume(s + Y)

		) + (

		l[(s - X) - Y] * geometry_type::InvVolume((s - X) - Y) +

		l[(s - X) + Y] * geometry_type::InvVolume((s - X) + Y) +

		l[(s + X) - Y] * geometry_type::InvVolume((s + X) - Y) +

		l[(s + X) + Y] * geometry_type::InvVolume((s + X) + Y)

		) * (

		r[s - Z] * geometry_type::InvVolume(s - Z) +

		r[s + Z] * geometry_type::InvVolume(s + Z)

		)) * 0.125 * geometry_type::Volume(s);
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, FACE, TL> const &l,
			Field<this_type, VERTEX, TR> const &r, index_type s) const ->decltype(l[s]*r[s])
	{
		auto Y = topology_type::DeltaIndex(topology_type::Roate(topology_type::Dual(s.self_)));
		auto Z = topology_type::DeltaIndex(topology_type::InverseRoate(topology_type::Dual(s.self_)));

		return l[s]
				* (

				r[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z)
						+ r[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z)
						+ r[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z)
						+ r[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

				) * 0.25;
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, FACE, TL> const &r,
			Field<this_type, EDGE, TR> const &l, index_type s) const ->decltype(l[s]*r[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return

		((

		r[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z) +

		r[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z) +

		r[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z) +

		r[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

		) * (

		l[s - X] * geometry_type::InvVolume(s - X) +

		l[s + X] * geometry_type::InvVolume(s + X)

		) + (

		r[(s - Z) - X] * geometry_type::InvVolume((s - Z) - X) +

		r[(s - Z) + X] * geometry_type::InvVolume((s - Z) + X) +

		r[(s + Z) - X] * geometry_type::InvVolume((s + Z) - X) +

		r[(s + Z) + X] * geometry_type::InvVolume((s + Z) + X)

		) * (

		l[s - Y] * geometry_type::InvVolume(s - Y) +

		l[s + Y] * geometry_type::InvVolume(s + Y)

		) + (

		r[(s - X) - Y] * geometry_type::InvVolume((s - X) - Y) +

		r[(s - X) + Y] * geometry_type::InvVolume((s - X) + Y) +

		r[(s + X) - Y] * geometry_type::InvVolume((s + X) - Y) +

		r[(s + X) + Y] * geometry_type::InvVolume((s + X) + Y)

		) * (

		l[s - Z] * geometry_type::InvVolume(s - Z) +

		l[s + Z] * geometry_type::InvVolume(s + Z)

		)) * 0.125 * geometry_type::Volume(s);
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<WEDGE>, Field<this_type, VOLUME, TL> const &l,
			Field<this_type, VERTEX, TR> const &r, index_type s) const ->decltype(r[s]*l[s])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return

		l[s] * (

		r[((s - X) - Y) - Z] * geometry_type::InvVolume(((s - X) - Y) - Z) +

		r[((s - X) - Y) + Z] * geometry_type::InvVolume(((s - X) - Y) + Z) +

		r[((s - X) + Y) - Z] * geometry_type::InvVolume(((s - X) + Y) - Z) +

		r[((s - X) + Y) + Z] * geometry_type::InvVolume(((s - X) + Y) + Z) +

		r[((s + X) - Y) - Z] * geometry_type::InvVolume(((s + X) - Y) - Z) +

		r[((s + X) - Y) + Z] * geometry_type::InvVolume(((s + X) - Y) + Z) +

		r[((s + X) + Y) - Z] * geometry_type::InvVolume(((s + X) + Y) - Z) +

		r[((s + X) + Y) + Z] * geometry_type::InvVolume(((s + X) + Y) + Z)

		) * 0.125;
	}

//***************************************************************************************************

	template<int IL, typename TL> inline auto OpEval(Int2Type<HODGESTAR>, Field<this_type, IL, TL> const & f,
			index_type s) const-> decltype(f[s]*1.0)
	{
//		auto X = topology_type::DeltaIndex(0,s.self_);
//		auto Y = topology_type::DeltaIndex(1,s.self_);
//		auto Z =topology_type::DeltaIndex(2,s.self_);
//
//		return
//
//		(
//
//		f[((s + X) - Y) - Z]*geometry_type::InvVolume(((s + X) - Y) - Z) +
//
//		f[((s + X) - Y) + Z]*geometry_type::InvVolume(((s + X) - Y) + Z) +
//
//		f[((s + X) + Y) - Z]*geometry_type::InvVolume(((s + X) + Y) - Z) +
//
//		f[((s + X) + Y) + Z]*geometry_type::InvVolume(((s + X) + Y) + Z) +
//
//		f[((s - X) - Y) - Z]*geometry_type::InvVolume(((s - X) - Y) - Z) +
//
//		f[((s - X) - Y) + Z]*geometry_type::InvVolume(((s - X) - Y) + Z) +
//
//		f[((s - X) + Y) - Z]*geometry_type::InvVolume(((s - X) + Y) - Z) +
//
//		f[((s - X) + Y) + Z]*geometry_type::InvVolume(((s - X) + Y) + Z)
//
//		) * 0.125 * geometry_type::Volume(s);

		return f[s] * geometry_type::InvVolume(s) * geometry_type::topology_type::DualVolume(s);
	}

	template<typename TL, typename TR> void OpEval(Int2Type<INTERIOR_PRODUCT>, nTuple<NDIMS, TR> const & v,
			Field<this_type, VERTEX, TL> const & f, index_type s) const = delete;

	template<typename TL, typename TR> inline auto OpEval(Int2Type<INTERIOR_PRODUCT>, nTuple<NDIMS, TR> const & v,
			Field<this_type, EDGE, TL> const & f, index_type s) const->decltype(f[s]*v[0])
	{
		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return

		(f[s + X] - f[s - X]) * 0.5 * v[0] +

		(f[s + Y] - f[s - Y]) * 0.5 * v[1] +

		(f[s + Z] - f[s - Z]) * 0.5 * v[2];
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<INTERIOR_PRODUCT>, nTuple<NDIMS, TR> const & v,
			Field<this_type, FACE, TL> const & f, index_type s) const->decltype(f[s]*v[0])
	{
		unsigned int n = topology_type::ComponentNum(s.self_);

		auto X = topology_type::DeltaIndex(s.self_);
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);
		return

		(f[s + Y] + f[s - Y]) * 0.5 * v[(n + 2) % 3] -

		(f[s + Z] + f[s - Z]) * 0.5 * v[(n + 1) % 3];
	}

	template<typename TL, typename TR> inline auto OpEval(Int2Type<INTERIOR_PRODUCT>, nTuple<NDIMS, TR> const & v,
			Field<this_type, VOLUME, TL> const & f, index_type s) const->decltype(f[s]*v[0])
	{
		unsigned int n = topology_type::ComponentNum(topology_type::Dual(s.self_));
		unsigned int D = topology_type::DeltaIndex(topology_type::Dual(s.self_));

		return (f[s + D] - f[s - D]) * 0.5 * v[n];
	}

//**************************************************************************************************
// Non-standard operation
// For curlpdx

	template<int N, typename TL> inline auto OpEval(Int2Type<EXTRIORDERIVATIVE>, Field<this_type, EDGE, TL> const & f,
			Int2Type<N>, index_type s) const-> decltype(f[s]-f[s])
	{

		auto X = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);

		Y = (topology_type::ComponentNum(Y) == N) ? Y : 0UL;
		Z = (topology_type::ComponentNum(Z) == N) ? Z : 0UL;

		return (f[s + Y] - f[s - Y]) - (f[s + Z] - f[s - Z]);
	}

	template<int N, typename TL> inline auto OpEval(Int2Type<CODIFFERENTIAL>, Field<this_type, FACE, TL> const & f,
			Int2Type<N>, index_type s) const-> decltype(f[s]-f[s])
	{

		auto X = topology_type::DeltaIndex(s.self_);
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);

		Y = (topology_type::ComponentNum(Y) == N) ? Y : 0UL;
		Z = (topology_type::ComponentNum(Z) == N) ? Z : 0UL;

		Real a = geometry_type::InvDualVolume(s) * geometry_type::Volume(s);

		return (

		f[s + Y] * (geometry_type::InvVolume(s + Y) * geometry_type::topology_type::DualVolume(s + Y) * a)

		- f[s - Y] * (geometry_type::InvVolume(s - Y) * geometry_type::topology_type::DualVolume(s - Y) * a)

		- f[s + Z] * (geometry_type::InvVolume(s + Z) * geometry_type::topology_type::DualVolume(s + Z) * a)

		+ f[s - Z] * (geometry_type::InvVolume(s - Z) * geometry_type::topology_type::DualVolume(s - Z) * a)

		);
	}
	template<int IL, typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<IL> const &,
			Field<this_type, IL, TR> const & f, index_type s) const
			DECL_RET_TYPE(f[s])

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<VERTEX> const &,
			Field<this_type, EDGE, TR> const & f, index_type s) const->nTuple<3,decltype(f[s]*1.0)>
	{

		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return nTuple<3, decltype(f[s]*1.0)>(
				{ (f[s - X] * geometry_type::InvVolume(s - X) + f[s + X] * geometry_type::InvVolume(s + X)) * 0.5
						* geometry_type::Volume(s),

				(f[s - Y] * geometry_type::InvVolume(s - Y) + f[s + Y] * geometry_type::InvVolume(s + Y)) * 0.5
						* geometry_type::Volume(s),

				(f[s - Z] * geometry_type::InvVolume(s - Z) + f[s + Z] * geometry_type::InvVolume(s + Z)) * 0.5
						* geometry_type::Volume(s) });
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<EDGE> const &,
			Field<this_type, VERTEX, TR> const & f, index_type s) const->decltype(f[s][0]*1.0)
	{

		auto n = topology_type::ComponentNum(s.self_);
		auto D = topology_type::DeltaIndex(s.self_);

		return ((f[s - D][n] * geometry_type::InvVolume(s - D) + f[s + D][n] * geometry_type::InvVolume(s + D)) * 0.5
				* geometry_type::Volume(s)

		);
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<VERTEX> const &,
			Field<this_type, FACE, TR> const & f, index_type s) const->nTuple<3,decltype(f[s]*1.0)>
	{

		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return nTuple<3, decltype(f[s]*1.0)>(
		{ (f[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z) +

		f[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z) +

		f[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z) +

		f[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

		) * 0.25 * geometry_type::Volume(s),

		(f[(s - Z) - X] * geometry_type::InvVolume((s - Z) - X) +

		f[(s - Z) + X] * geometry_type::InvVolume((s - Z) + X) +

		f[(s + Z) - X] * geometry_type::InvVolume((s + Z) - X) +

		f[(s + Z) + X] * geometry_type::InvVolume((s + Z) + X)

		) * 0.25 * geometry_type::Volume(s),

		(f[(s - X) - Y] * geometry_type::InvVolume((s - X) - Y) +

		f[(s - X) + Y] * geometry_type::InvVolume((s - X) + Y) +

		f[(s + X) - Y] * geometry_type::InvVolume((s + X) - Y) +

		f[(s + X) + Y] * geometry_type::InvVolume((s + X) + Y)

		) * 0.25 * geometry_type::Volume(s),

		});
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<FACE> const &,
			Field<this_type, VERTEX, TR> const & f, index_type s) const->decltype(f[s][0]*1.0)
	{

		auto n = topology_type::ComponentNum(topology_type::Dual(s.self_));
		auto X = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);

		return (

		(f[(s - Y) - Z][n] * geometry_type::InvVolume((s - Y) - Z) +

		f[(s - Y) + Z][n] * geometry_type::InvVolume((s - Y) + Z) +

		f[(s + Y) - Z][n] * geometry_type::InvVolume((s + Y) - Z) +

		f[(s + Y) + Z][n] * geometry_type::InvVolume((s + Y) + Z)

		) * 0.25 * geometry_type::Volume(s)

		);
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<VOLUME>, Field<this_type, FACE, TR> const & f,
			index_type s) const->nTuple<3,decltype(f[s]*1.0)>
	{

		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return nTuple<3, decltype(f[s]*1.0)>(
				{ (f[s - X] * geometry_type::InvVolume(s - X) + f[s + X] * geometry_type::InvVolume(s + X)) * 0.5
						* geometry_type::Volume(s),

				(f[s - Y] * geometry_type::InvVolume(s - Y) + f[s + Y] * geometry_type::InvVolume(s + Y)) * 0.5
						* geometry_type::Volume(s),

				(f[s - Z] * geometry_type::InvVolume(s - Z) + f[s + Z] * geometry_type::InvVolume(s + Z)) * 0.5
						* geometry_type::Volume(s) });
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<FACE>, Field<this_type, VOLUME, TR> const & f,
			index_type s) const->decltype(f[s][0]*1.0)
	{

		auto n = topology_type::ComponentNum(topology_type::Dual(s.self_));
		auto D = topology_type::DeltaIndex(topology_type::Dual(s.self_));

		return ((f[s - D][n] * geometry_type::InvVolume(s - D) + f[s + D][n] * geometry_type::InvVolume(s + D)) * 0.5
				* geometry_type::Volume(s)

		);
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<VOLUME>, Field<this_type, EDGE, TR> const & f,
			index_type s) const->nTuple<3,decltype(f[s]*1.0)>
	{

		auto X = topology_type::DeltaIndex(0, s.self_);
		auto Y = topology_type::DeltaIndex(1, s.self_);
		auto Z = topology_type::DeltaIndex(2, s.self_);

		return nTuple<3, decltype(f[s]*1.0)>(
		{ (f[(s - Y) - Z] * geometry_type::InvVolume((s - Y) - Z) +

		f[(s - Y) + Z] * geometry_type::InvVolume((s - Y) + Z) +

		f[(s + Y) - Z] * geometry_type::InvVolume((s + Y) - Z) +

		f[(s + Y) + Z] * geometry_type::InvVolume((s + Y) + Z)

		) * 0.25 * geometry_type::Volume(s),

		(f[(s - Z) - X] * geometry_type::InvVolume((s - Z) - X) +

		f[(s - Z) + X] * geometry_type::InvVolume((s - Z) + X) +

		f[(s + Z) - X] * geometry_type::InvVolume((s + Z) - X) +

		f[(s + Z) + X] * geometry_type::InvVolume((s + Z) + X)

		) * 0.25 * geometry_type::Volume(s),

		(f[(s - X) - Y] * geometry_type::InvVolume((s - X) - Y) +

		f[(s - X) + Y] * geometry_type::InvVolume((s - X) + Y) +

		f[(s + X) - Y] * geometry_type::InvVolume((s + X) - Y) +

		f[(s + X) + Y] * geometry_type::InvVolume((s + X) + Y)

		) * 0.25 * geometry_type::Volume(s),

		});
	}

	template<typename TR> inline auto OpEval(Int2Type<MAPTO>, Int2Type<EDGE>, Field<this_type, VOLUME, TR> const & f,
			index_type s) const->decltype(f[s][0]*1.0)
	{

		auto n = topology_type::ComponentNum(topology_type::Dual(s.self_));
		auto X = topology_type::DeltaIndex(topology_type::Dual(s.self_));
		auto Y = topology_type::Roate(X);
		auto Z = topology_type::InverseRoate(X);
		return (

		(f[(s - Y) - Z][n] * geometry_type::InvVolume((s - Y) - Z) +

		f[(s - Y) + Z][n] * geometry_type::InvVolume((s - Y) + Z) +

		f[(s + Y) - Z][n] * geometry_type::InvVolume((s + Y) - Z) +

		f[(s + Y) + Z][n] * geometry_type::InvVolume((s + Y) + Z)

		) * 0.25 * geometry_type::Volume(s)

		);
	}
};

}
// namespace simpla

#endif /* MESH_RECTANGLE_H_ */
