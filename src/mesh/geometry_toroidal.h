/*
 * geometry_toroidal.h
 *
 *  Created on: 2014年3月13日
 *      Author: salmon
 */

#ifndef GEOMETRY_TOROIDAL_H_
#define GEOMETRY_TOROIDAL_H_

#include <iostream>
#include <utility>

#include "../fetl/ntuple.h"
#include "../fetl/primitives.h"
#include "../utilities/log.h"

namespace simpla
{

/**
 *  R psi phi
 */
template<typename Topology>
struct ToroidalGeometry
{
	typedef Topology topology_type;

	typedef ToroidalGeometry<topology_type> this_type;

	static constexpr int NDIMS = topology_type::NDIMS;

	typedef typename topology_type::coordinates_type coordinates_type;
	typedef typename topology_type::index_type index_type;

	typedef nTuple<NDIMS, Real> vector_type;
	typedef nTuple<NDIMS, Real> covector_type;

	topology_type const & topology;

	ToroidalGeometry(this_type const & rhs) = delete;

	ToroidalGeometry(topology_type const & t)
			: topology(t)
	{
		UNIMPLEMENT;
	}
	template<typename TDict>
	ToroidalGeometry(topology_type const & t, TDict const & dict)
			: topology(t)
	{

	}

	~ToroidalGeometry()
	{

	}

	//***************************************************************************************************
	// Geometric properties
	// Metric
	//***************************************************************************************************

	void Update()
	{

	}
	template<typename TDict>
	void Load(TDict const & dict)
	{

	}

	std::ostream & Save(std::ostream & os) const
	{
		return os;
	}

	coordinates_type xmin_ = { 0, 0, 0 };

	coordinates_type xmax_ = { 1, 1, 1 };

	coordinates_type inv_L = { 1.0, 1.0, 1.0 };

	coordinates_type scale_ = { 1.0, 1.0, 1.0 };

	coordinates_type inv_scale_ = { 1.0, 1.0, 1.0 };

	coordinates_type shift_ = { 0, 0, 0 };

	static constexpr nTuple<NDIMS, Real> normal_[NDIMS] = {

	1, 0, 0,

	0, 1, 0,

	0, 0, 1

	};

	/**
	 *
	 *                ^y
	 *               /
	 *        z     /
	 *        ^    /
	 *        |  110-------------111
	 *        |  /|              /|
	 *        | / |             / |
	 *        |/  |            /  |
	 *       100--|----------101  |
	 *        | m |           |   |
	 *        |  010----------|--011
	 *        |  /            |  /
	 *        | /             | /
	 *        |/              |/
	 *       000-------------001---> x
	 *
	 *
	 */
	Real dh_[NDIMS] = { 1, 1, 1 };

	Real inv_volume_[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	nTuple<NDIMS, Real> dx_;

	nTuple<NDIMS, Real> const & GetDx() const
	{
		return dx_;
	}

	template<int IN, typename T>
	inline void SetExtent(nTuple<IN, T> const & pmin, nTuple<IN, T> const & pmax)
	{
		int n = IN < NDIMS ? IN : NDIMS;

		auto const & dims = topology.GetDimensions();

		for (int i = 0; i < n; ++i)
		{
			xmin_[i] = pmin[i];
			xmax_[i] = pmax[i];

			shift_[i] = pmin[i];

			scale_[i] = (pmax[i] > pmin[i]) ? ((static_cast<Real>(dims[i])) / (pmax[i] - pmin[i])) : 0;
			inv_scale_[i] = (pmax[i] - pmin[i]) / (static_cast<Real>(dims[i]));
		}

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^    /
		 *        |  110-------------111
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *       100--|----------101  |
		 *        | m |           |   |
		 *        |  010----------|--011
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 *       000-------------001---> x
		 *
		 *
		 */

		dh_[0] /* R   */= (dims[0] > 1) ? (xmax_[0] - xmin_[0]) / dims[0] : 1;
		dh_[1] /* Z   */= (dims[1] > 1) ? (xmax_[1] - xmin_[1]) / dims[1] : 1;
		dh_[2] /* Phi */= (dims[2] > 1) ? (xmax_[2] - xmin_[2]) / dims[2] : 1;

	}

	inline std::pair<coordinates_type, coordinates_type> GetExtent() const
	{
		return std::move(std::make_pair(xmin_, xmax_));
	}

	inline coordinates_type GetCoordinates(coordinates_type const &x) const
	{
		return coordinates_type( {

		xmin_[0] + (xmax_[0] - xmin_[0]) * x[0],

		xmin_[1] + (xmax_[1] - xmin_[1]) * x[1],

		xmin_[2] + (xmax_[2] - xmin_[2]) * x[2]

		});
	}

	nTuple<3, Real> const& Normal(index_type s) const
	{
		return normal_[topology.topology_type::_C(s)];
	}

	template<typename TV>
	TV const& Normal(index_type s, nTuple<3, TV> const & v) const
	{
		return v[topology.topology_type::_C(s)];
	}

	Real Volume(index_type s) const
	{
		auto x = topology.GetCoordinates(s);
		Real res = 1;

		switch (topology._N(s))
		{
		case 0:
			res = 1;
			break;
		case 1:  //001 r
			res = dh_[0];
			break;
		case 2:  //010 z
			res = dh_[1];
			break;
		case 4:  //100 phi
			res = x[0] * x[2];
			break;
		case 3: //011 rz
			res = dh_[0] * dh_[1];
			break;
		case 5: //101 r phi  phi*( (r+d)^2-r^2)/2
			res = x[2] * (2 * x[0] + dh_[1]) * dh_[1] * 0.5;
			break;
		case 6: //110 z phi
			res = dh_[2] * x[0] * x[2];
			break;
		case 7: //111
			res = x[2] * (2 * x[0] + dh_[0]) * 0.5 * dh_[0] * dh_[1];
			break;
		}

		return res;
	}
	Real InvVolume(index_type s) const
	{
		return 1.0 / Volume(s);
	}
	coordinates_type Map(coordinates_type const &x) const
	{
		return coordinates_type( {

		(x[0] - shift_[0]) * scale_[0],

		(x[0] - shift_[1]) * scale_[1],

		(x[0] - shift_[2]) * scale_[2]

		});
	}
	coordinates_type InverseMap(coordinates_type const &x) const
	{
		return coordinates_type( {

		x[0] * inv_scale_[0] + shift_[0],

		x[1] * inv_scale_[1] + shift_[1],

		x[2] * inv_scale_[2] + shift_[2]

		});
	}

	template<typename TV>
	nTuple<NDIMS, TV> const& PushForward(index_type s, nTuple<NDIMS, TV> const & v) const
	{
		return v;
	}
	template<typename TV>
	nTuple<NDIMS, TV> const& PullBack(index_type s, nTuple<NDIMS, TV> const & v) const
	{
		return v;
	}
}
;

}  // namespace simpla

#endif /* GEOMETRY_TOROIDAL_H_ */