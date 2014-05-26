/*
 * geometry_euclidean.h
 *
 *  Created on: 2014年3月13日
 *      Author: salmon
 */

#ifndef GEOMETRY_EUCLIDEAN_H_
#define GEOMETRY_EUCLIDEAN_H_

#include <iostream>
#include <utility>

#include "../fetl/ntuple.h"
#include "../fetl/primitives.h"

namespace simpla
{
template<typename TTopology>
struct EuclideanGeometry: public TTopology
{
	typedef TTopology topology_type;

	typedef EuclideanGeometry<topology_type> this_type;

	static constexpr int NDIMS = topology_type::NDIMS;

	typedef typename topology_type::coordinates_type coordinates_type;
	typedef typename topology_type::iterator index_type;
	typedef typename topology_type::size_type size_type;
	typedef nTuple<NDIMS, Real> vector_type;
	typedef nTuple<NDIMS, Real> covector_type;

	EuclideanGeometry(this_type const & rhs) = delete;

	EuclideanGeometry()
			: topology_type()
	{

	}
	template<typename TDict>
	EuclideanGeometry(TDict const & dict)
			: topology_type(dict)
	{
		Load(dict);
	}

	~EuclideanGeometry()
	{
	}

	//***************************************************************************************************
	// Geometric properties
	// Metric
	//***************************************************************************************************

	Real dt_ = 0.0;
	Real time0_ = 0.0;
	// Time
	void NextTimeStep()
	{
		topology_type::NextTimeStep();
	}
	Real GetTime() const
	{
		return static_cast<double>(topology_type::GetClock()) * dt_ + time0_;
	}

	Real GetDt() const
	{
		return dt_;
	}

	coordinates_type xmin_ = { 0, 0, 0 };

	coordinates_type xmax_ = { 1, 1, 1 };

	coordinates_type inv_dx_ = { 1.0, 1.0, 1.0 };

	coordinates_type dx_ = { 1.0, 1.0, 1.0 };

	coordinates_type shift_ = { 0, 0, 0 };

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

	Real volume_[8] = { 1, // 000
	        1, //001
	        1, //010
	        1, //011
	        1, //100
	        1, //101
	        1, //110
	        1  //111
	        };
	Real inv_volume_[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	Real dual_volume_[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	Real inv_dual_volume_[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	template<typename TDict, typename ...Others>
	void Load(TDict const & dict, Others const &...)
	{
		if (dict["Min"] && dict["Max"])
		{
			LOGGER << "Load EuclideanGeometry ";

			SetExtents(

			dict["Dimensions"].template as<nTuple<NDIMS, size_type>>(),

			dict["Min"].template as<nTuple<NDIMS, Real>>(),

			dict["Max"].template as<nTuple<NDIMS, Real>>());
		}
	}

	std::string Save(std::string const &path) const
	{
		std::stringstream os;

		os << "\tMin = " << xmin_ << " , " << "Max  = " << xmax_;

		return os.str();
	}

	inline void SetExtents(nTuple<NDIMS, size_type> dims, nTuple<NDIMS, Real> pmin, nTuple<NDIMS, Real> pmax)
	{

		for (int i = 0; i < NDIMS; ++i)
		{
			xmin_[i] = pmin[i];

			shift_[i] = xmin_[i];

			if ((std::abs(pmax[i] - pmin[i]) < EPSILON) || (dims[i] <= 1))
			{

				xmax_[i] = xmin_[i];

				dims[i] = 1;

				inv_dx_[i] = 0.0;

				dx_[i] = 0.0;

				volume_[1UL << i] = 1.0;

				dual_volume_[7 - (1UL << i)] = 1.0;

				inv_volume_[1UL << i] = 1.0;

				inv_dual_volume_[7 - (1UL << i)] = 1.0;

			}
			else
			{
				xmax_[i] = pmax[i];

				inv_dx_[i] = static_cast<Real>(dims[i]) / (xmax_[i] - xmin_[i]);

				dx_[i] = (xmax_[i] - xmin_[i]) / static_cast<Real>(dims[i]);

				volume_[1UL << i] = dx_[i];

				dual_volume_[7 - (1UL << i)] = dx_[i];

				inv_volume_[1UL << i] = inv_dx_[i];

				inv_dual_volume_[7 - (1UL << i)] = inv_dx_[i];

			}
		}

		topology_type::SetDimensions(dims);

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

		volume_[0] = 1;
//		volume_[1] /* 001 */= dx_[0];
//		volume_[2] /* 010 */= dx_[1];
//		volume_[4] /* 100 */= dx_[2];

		volume_[3] /* 011 */= volume_[1] * volume_[2];
		volume_[5] /* 101 */= volume_[4] * volume_[1];
		volume_[6] /* 110 */= volume_[2] * volume_[4];

		volume_[7] /* 111 */= volume_[1] * volume_[2] * volume_[4];

		dual_volume_[7] = 1;
//		dual_volume_[6] /* 001 */= dx_[0];
//		dual_volume_[5] /* 010 */= dx_[1];
//		dual_volume_[3] /* 100 */= dx_[2];

		dual_volume_[4] /* 011 */= dual_volume_[6] * dual_volume_[5];
		dual_volume_[2] /* 101 */= dual_volume_[3] * dual_volume_[6];
		dual_volume_[1] /* 110 */= dual_volume_[5] * dual_volume_[3];

		dual_volume_[0] /* 111 */= dual_volume_[6] * dual_volume_[5] * dual_volume_[3];

		inv_volume_[0] = 1;
//		inv_volume_[1] /* 001 */= inv_dx_[0];
//		inv_volume_[2] /* 010 */= inv_dx_[1];
//		inv_volume_[4] /* 100 */= inv_dx_[2];

		inv_volume_[3] /* 011 */= inv_volume_[1] * inv_volume_[2];
		inv_volume_[5] /* 101 */= inv_volume_[4] * inv_volume_[1];
		inv_volume_[6] /* 110 */= inv_volume_[2] * inv_volume_[4];

		inv_volume_[7] /* 111 */= inv_volume_[1] * inv_volume_[2] * inv_volume_[4];

		inv_dual_volume_[7] = 1;
//		inv_dual_volume_[6] /* 001 */= inv_dx_[0];
//		inv_dual_volume_[5] /* 010 */= inv_dx_[1];
//		inv_dual_volume_[3] /* 100 */= inv_dx_[2];

		inv_dual_volume_[4] /* 011 */= inv_dual_volume_[6] * inv_dual_volume_[5];
		inv_dual_volume_[2] /* 101 */= inv_dual_volume_[3] * inv_dual_volume_[6];
		inv_dual_volume_[1] /* 110 */= inv_dual_volume_[5] * inv_dual_volume_[3];

		inv_dual_volume_[0] /* 111 */= inv_dual_volume_[6] * inv_dual_volume_[5] * inv_dual_volume_[3];

	}

	inline std::pair<coordinates_type, coordinates_type> GetExtents() const
	{
		return std::move(std::make_pair(xmin_, xmax_));
	}

	inline coordinates_type const & GetDx() const
	{
		return dx_;
	}

	template<typename ... Args>
	inline coordinates_type GetCoordinates(Args const & ... args) const
	{
		return CoordinatesFromTopology(topology_type::GetCoordinates(std::forward<Args const &>(args)...));
	}

	coordinates_type CoordinatesFromTopology(coordinates_type const &x) const
	{
		return coordinates_type( {

		x[0] * dx_[0] + shift_[0],

		x[1] * dx_[1] + shift_[1],

		x[2] * dx_[2] + shift_[2]

		});

	}
	coordinates_type CoordinatesToTopology(coordinates_type const &x) const
	{
		return coordinates_type( {

		(x[0] - shift_[0]) * inv_dx_[0],

		(x[1] - shift_[1]) * inv_dx_[1],

		(x[2] - shift_[2]) * inv_dx_[2]

		});

	}
	template<typename ... Args>
	inline coordinates_type CoordinatesLocalToGlobal(Args const & ... args) const
	{
		return CoordinatesFromTopology(topology_type::CoordinatesLocalToGlobal(std::forward<Args const &>(args)...));
	}

	index_type CoordinatesGlobalToLocal(coordinates_type * px) const
	{
		*px = CoordinatesToTopology(*px);
		return topology_type::CoordinatesGlobalToLocal(px);
	}

	coordinates_type CoordinatesToCartesian(coordinates_type const &x) const
	{
		return x;
	}

	coordinates_type CoordinatesFromCartesian(coordinates_type const &x) const
	{
		return x;
	}

	template<typename TV>
	nTuple<NDIMS, TV> const& PushForward(coordinates_type const &x, nTuple<NDIMS, TV> const & v) const
	{
		return v;
	}
	template<typename TV>
	nTuple<NDIMS, TV> const& PullBack(coordinates_type const &x, nTuple<NDIMS, TV> const & v) const
	{
		return v;
	}

	template<typename TV>
	TV const& Normal(index_type s, TV const & v) const
	{
		return v;
	}

	template<typename TV>
	TV const& Normal(index_type s, nTuple<3, TV> const & v) const
	{
		return v[s.Component()];
	}

	template<typename TV>
	TV Sample(Int2Type<VERTEX>, index_type s, TV const &v) const
	{
		return v;
	}

	template<typename TV>
	TV Sample(Int2Type<VOLUME>, index_type s, TV const &v) const
	{
		return v;
	}

	template<typename TV>
	TV Sample(Int2Type<EDGE>, index_type s, nTuple<3, TV> const &v) const
	{
		return v[topology_type::Component(s.self_)];
	}

	template<typename TV>
	TV Sample(Int2Type<FACE>, index_type s, nTuple<3, TV> const &v) const
	{
		return v[topology_type::Component(s.self_)];
	}

	template<int IFORM, typename TV>
	TV Sample(Int2Type<IFORM>, index_type s, TV const & v) const
	{
		return v;
	}

	template<int IFORM, typename TV>
	typename std::enable_if<(IFORM == EDGE || IFORM == FACE), TV>::type Sample(Int2Type<IFORM>, index_type s,
	        nTuple<NDIMS, TV> const & v) const
	{
		return Normal(s, v);
	}

//***************************************************************************************************
// Cell-wise operation
//***************************************************************************************************
	Real Volume(index_type s) const
	{
		return topology_type::Volume(s) * volume_[topology_type::NodeId(s.self_)];
	}
	Real InvVolume(index_type s) const
	{
		return topology_type::InvVolume(s) * inv_volume_[topology_type::NodeId(s.self_)];
	}

	Real DualVolume(index_type s) const
	{
		return topology_type::DualVolume(s) * dual_volume_[topology_type::NodeId(s.self_)];
	}
	Real InvDualVolume(index_type s) const
	{
		return topology_type::InvDualVolume(s) * inv_dual_volume_[topology_type::NodeId(s.self_)];
	}

}
;

}  // namespace simpla

#endif /* GEOMETRY_EUCLIDEAN_H_ */
