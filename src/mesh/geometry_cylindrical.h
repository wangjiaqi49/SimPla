/*
 * geometry_cylindrical.h
 *
 *  Created on: 2014-3-13
 *      Author: salmon
 */

#ifndef GEOMETRY_CYLINDRICAL_H_
#define GEOMETRY_CYLINDRICAL_H_

#include <iostream>
#include <utility>
#include <cmath>
#include "../utilities/ntuple.h"
#include "../utilities/primitives.h"
#include "../utilities/log.h"
#include "../physics/constants.h"

namespace simpla
{

/**
 *  \ingroup Geometry
 *
 *  \brief  cylindrical geometry (R Z phi)
 */
template<typename TTopology, unsigned int IPhiAxis = 2>
struct CylindricalGeometry: public TTopology
{
	typedef TTopology topology_type;

	static constexpr unsigned int PhiAxis = (IPhiAxis) % 3;
	static constexpr unsigned int RAxis = (PhiAxis + 1) % 3;
	static constexpr unsigned int ZAxis = (PhiAxis + 2) % 3;

	typedef CylindricalGeometry<topology_type, PhiAxis> this_type;

	static constexpr unsigned int NDIMS = topology_type::NDIMS;

	typedef Real scalar_type;

	typedef typename topology_type::coordinates_type coordinates_type;
	typedef typename topology_type::index_type index_type;
	typedef typename topology_type::compact_index_type compact_index_type;
	typedef typename topology_type::iterator iterator;

	typedef nTuple<NDIMS, Real> vector_type;
	typedef nTuple<NDIMS, Real> covector_type;

	CylindricalGeometry(this_type const & rhs) = delete;

	CylindricalGeometry() :
			topology_type()
	{

	}
	template<typename TDict>
	CylindricalGeometry(TDict const & dict) :
			topology_type(dict)
	{
		Load(dict);
	}

	~CylindricalGeometry()
	{
	}
	static std::string TypeAsString()
	{
		return "Cylindrical";
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
	Real get_time() const
	{
		return static_cast<double>(topology_type::get_clock()) * dt_ + time0_;
	}

	Real get_dt() const
	{
		return dt_;
	}

	coordinates_type xmin_ = { 1, 0, 0 };

	coordinates_type xmax_ = { 2, 1, TWOPI };

	coordinates_type inv_length_ = { 1.0, 1.0, 1.0 / TWOPI };

	coordinates_type length_ = { 2, 1, TWOPI };

	coordinates_type shift_ = { 0, 0, 0 };

	template<typename TDict, typename ...Others>
	void Load(TDict const & dict, Others &&...others)
	{
		try
		{
			topology_type::Load(dict, std::forward<Others >(others)...);

			if (dict["Min"] && dict["Max"])
			{
				LOGGER << "Load CylindricalGeometry ";

				set_extents(

				dict["Min"].template as<nTuple<NDIMS, Real>>(),

				dict["Max"].template as<nTuple<NDIMS, Real>>());
			}

			dt_ = dict["dt"].template as<Real>();

		}
		catch (...)
		{
			PARSER_ERROR("Configure CylindricalGeometry error!");
		}
	}

	template<typename OS>
	OS & Print(OS &os) const
	{
		topology_type::Print(os) << std::endl

		<< " , " << " Min = " << xmin_ << " , " << "Max  = " << xmax_ << ", " << " dt  = " << dt_;

		return os;
	}

	std::string Save(std::string const &path) const
	{
		return path;
	}

	template<typename ...Others>
	inline void set_extents(coordinates_type const & pmin, coordinates_type const & pmax, Others&& ... others)
	{
		topology_type::set_dimensions(std::forward<Others >(others)...);
		set_extents(pmin, pmax);
	}
	void set_extents(nTuple<NDIMS, Real> const & pmin, nTuple<NDIMS, Real> const & pmax)
	{
		auto dims = topology_type::get_dimensions();

		if (pmin[RAxis] < EPSILON || dims[RAxis] <= 1)
		{

			RUNTIME_ERROR(

			std::string(" illegal configure: Cylindrical R_min=0 or dims[R]<=1!!")

			+ " coordinates = ("

			+ " R=[ " + ToString(pmin[RAxis]) + " , " + ToString(pmax[RAxis]) + "]"

			+ ", Z=[ " + ToString(pmin[ZAxis]) + " , " + ToString(pmax[ZAxis]) + "]"

			+ ", Phi=[ " + ToString(pmin[PhiAxis]) + " , " + ToString(pmax[PhiAxis]) + "] )"

			+ ", dimension = ("

			+ " R = " + ToString(dims[RAxis])

			+ ", Z = " + ToString(dims[RAxis])

			+ ", Phi =" + ToString(dims[PhiAxis]) + ")"

			);
		}

		for (int i = 0; i < NDIMS; ++i)
		{
			xmin_[i] = pmin[i];

			shift_[i] = xmin_[i];

			if ((pmax[i] - pmin[i]) < EPSILON || dims[i] <= 1)
			{

				xmax_[i] = xmin_[i];

				inv_length_[i] = 0.0;

				length_[i] = 0.0;

			}
			else
			{
				xmax_[i] = pmax[i];

				length_[i] = (xmax_[i] - xmin_[i]);

				if (i == PhiAxis && length_[i] > TWOPI)
				{
					xmax_[i] = xmin_[i] + TWOPI;

					length_[i] = TWOPI;
				}

				inv_length_[i] = 1.0 / length_[i];

			}
		}

		UpdateVolume();
	}
	inline std::pair<coordinates_type, coordinates_type> get_extents() const
	{
		return std::move(std::make_pair(xmin_, xmax_));
	}

	inline coordinates_type get_dx(compact_index_type s = 0UL) const
	{

		coordinates_type res;

		auto d = topology_type::get_dx();

		for (int i = 0; i < NDIMS; ++i)
		{
			res[i] = length_[i] * d[i];
		}

		return std::move(res);
	}
	//! @name Normalize coordiantes to  [0,1 )
	//!@{

	template<typename ... Args>
	inline coordinates_type get_coordinates(Args && ... args) const
	{
		return CoordinatesFromTopology(topology_type::get_coordinates(std::forward<Args >(args)...));
	}

	coordinates_type CoordinatesFromTopology(coordinates_type const &x) const
	{

		return coordinates_type( {

		x[0] * length_[0] + shift_[0],

		x[1] * length_[1] + shift_[1],

		x[2] * length_[2] + shift_[2]

		});

	}
	coordinates_type CoordinatesToTopology(coordinates_type const &x) const
	{
		return coordinates_type( {

		(x[0] - shift_[0]) * inv_length_[0],

		(x[1] - shift_[1]) * inv_length_[1],

		(x[2] - shift_[2]) * inv_length_[2]

		});

	}
	template<typename ... Args>
	inline coordinates_type CoordinatesLocalToGlobal(Args && ... args) const
	{
		return CoordinatesFromTopology(topology_type::CoordinatesLocalToGlobal(std::forward<Args >(args)...));
	}

	std::tuple<compact_index_type, coordinates_type> CoordinatesGlobalToLocal(coordinates_type x,
	        typename topology_type::compact_index_type shift = 0UL) const
	{
		return std::move(topology_type::CoordinatesGlobalToLocal(std::move(CoordinatesToTopology(x)), shift));
	}

	//!@}
	//! @name Coordiantes convert Cylindrical <-> Cartesian
	//! \f$\left(r,z,\phi\right)\Longleftrightarrow\left(x,y,z\right)\f$
	//! @{

	coordinates_type InvMapTo(coordinates_type const &r, unsigned int CartesianZAxis = 2) const
	{
		coordinates_type x;

		/**
		 *  @note
		 * coordinates transforam
		 *
		 *  \f{eqnarray*}{
		 *		x & = & r\cos\phi\\
		 *		y & = & r\sin\phi\\
		 *		z & = & Z
		 *  \f}
		 *
		 */
		x[(CartesianZAxis + 1) % 3] = r[RAxis] * std::cos(r[PhiAxis]);
		x[(CartesianZAxis + 2) % 3] = r[RAxis] * std::sin(r[PhiAxis]);
		x[(CartesianZAxis + 3) % 3] = r[ZAxis];

		return std::move(x);
	}

	coordinates_type MapTo(coordinates_type const &x, unsigned int CartesianZAxis = 2) const
	{
		coordinates_type r;
		/**
		 *  @note
		 *  coordinates transforam
		 *  \f{eqnarray*}{
		 *		r&=&\sqrt{x^{2}+y^{2}}\\
		 *		Z&=&z\\
		 *		\phi&=&\arg\left(x,y\right)
		 *  \f}
		 *
		 */
		r[ZAxis] = x[CartesianZAxis];
		r[RAxis] = std::sqrt(
		        x[(CartesianZAxis + 1) % 3] * x[(CartesianZAxis + 1) % 3]
		                + x[(CartesianZAxis + 2) % 3] * x[(CartesianZAxis + 2) % 3]);
		r[PhiAxis] = std::atan2(x[(CartesianZAxis + 2) % 3], x[(CartesianZAxis + 1) % 3]);

		return r;
	}

	template<typename TV>
	std::tuple<coordinates_type, TV> PushForward(std::tuple<coordinates_type, TV> const & Z,
	        unsigned int CartesianZAxis = 2) const
	{
		return std::move(std::make_tuple(MapTo(std::get<0>(Z), CartesianZAxis), std::get<1>(Z)));
	}

	template<typename TV>
	std::tuple<coordinates_type, TV> PullBack(std::tuple<coordinates_type, TV> const & R, unsigned int CartesianZAxis =
	        2) const
	{
		return std::move(std::make_tuple(InvMapTo(std::get<0>(R), CartesianZAxis), std::get<1>(R)));
	}

	/**
	 *
	 *   transform vector from Cartesian to Cylindrical
	 *
	 *
	 * \verbatim
	 *
	 *     theta   y   r
	 *          \  |  /
	 *           \ | /
	 *            \|/------x
	 *          y  /
	 *          | /
	 *          |/)theta
	 *          0------x
	 *
	 * \endverbatim
	 *
	 * @param Z  \f$ \left(x,y,z\right),u=u_{x}\partial_{x}+u_{y}\partial_{y}+u_{z}\partial_{z} \f$
	 * @param CartesianZAxis
	 * @return  \f$ v=v_{r}\partial_{r}+v_{Z}\partial_{Z}+v_{\theta}/r\partial_{\theta} \f$
	 *
	 */
	template<typename TV>
	std::tuple<coordinates_type, nTuple<NDIMS, TV> > PushForward(
	        std::tuple<coordinates_type, nTuple<NDIMS, TV> > const & Z, unsigned int CartesianZAxis = 2) const
	{
		coordinates_type r = MapTo(std::get<0>(Z), CartesianZAxis);

		auto const & v = std::get<1>(Z);

		nTuple<NDIMS, TV> u;

		Real c = std::cos(r[PhiAxis]), s = std::sin(r[PhiAxis]);

		u[ZAxis] = v[CartesianZAxis % 3];

		u[RAxis] = v[(CartesianZAxis + 1) % 3] * c + v[(CartesianZAxis + 2) % 3] * s;

		u[PhiAxis] = (-v[(CartesianZAxis + 1) % 3] * s + v[(CartesianZAxis + 2) % 3] * c) / r[RAxis];

		return std::move(std::make_tuple(r, u));
	}
	/**
	 *
	 *   PullBack vector from Cylindrical  to Cartesian
	 *
	 *
	 * @param R  \f$ v=v_{r}\partial_{r}+v_{Z}\partial_{Z}+v_{\theta}/r\partial_{\theta} \f$
	 * @param CartesianZAxis
	 * @return  \f$ \left(x,y,z\right),u=u_{x}\partial_{x}+u_{y}\partial_{y}+u_{z}\partial_{z} \f$
	 *
	 */
	template<typename TV>
	std::tuple<coordinates_type, nTuple<NDIMS, TV> > PullBack(
	        std::tuple<coordinates_type, nTuple<NDIMS, TV> > const & R, unsigned int CartesianZAxis = 2) const
	{
		auto const & r = std::get<0>(R);
		auto const & u = std::get<1>(R);

		Real c = std::cos(r[PhiAxis]), s = std::sin(r[PhiAxis]);

		nTuple<NDIMS, TV> v;

		v[(CartesianZAxis + 1) % 3] = u[RAxis] * c - u[PhiAxis] * r[RAxis] * s;
		v[(CartesianZAxis + 2) % 3] = u[RAxis] * s + u[PhiAxis] * r[RAxis] * c;
		v[(CartesianZAxis + 3) % 3] = u[ZAxis];

		return std::move(std::make_tuple(InvMapTo(r), v));
	}
	//! @}

	auto Select(unsigned int iform, coordinates_type const & xmin, coordinates_type const & xmax) const
	DECL_RET_TYPE((topology_type::Select(iform, CoordinatesToTopology(xmin),CoordinatesToTopology(xmax))))

	template<typename ...Args>
	auto Select(unsigned int iform, Args && ...args) const
	DECL_RET_TYPE((topology_type::Select(iform,std::forward<Args >(args)...)))

	template<typename TV>
	TV const& Normal(index_type s, TV const & v) const
	{
		return v;
	}

	template<typename TV>
	TV const& Normal(index_type s, nTuple<3, TV> const & v) const
	{
		return v[topology_type::ComponentNum(s)];
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
		return std::get<1>(PushForward(std::make_tuple(get_coordinates(s), v)))[topology_type::ComponentNum(s)];
	}

	template<typename TV>
	TV Sample(Int2Type<FACE>, index_type s, nTuple<3, TV> const &v) const
	{
		return std::get<1>(PushForward(std::make_tuple(get_coordinates(s), v)))[topology_type::ComponentNum(s)];
	}

	template<unsigned int IFORM, typename TV>
	TV Sample(Int2Type<IFORM>, index_type s, TV const & v) const
	{
		return v;
	}

	template<unsigned int IFORM, typename TV>
	typename std::enable_if<(IFORM == EDGE || IFORM == FACE), TV>::type Sample(Int2Type<IFORM>, index_type s,
	        nTuple<NDIMS, TV> const & v) const
	{
		return Normal(s, v);
	}

	//! @name Matric/coordinates  depend transform
	//! @{

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

	/**
	 *\verbatim
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
	 *\endverbatim
	 */
	void UpdateVolume()
	{

		for (int i = 0; i < NDIMS; ++i)
		{

			if ((xmax_[i] - xmin_[i]) < EPSILON)
			{

				volume_[1UL << (NDIMS - i - 1)] = 1.0;

				inv_volume_[1UL << (NDIMS - i - 1)] = 1.0;
			}
			else
			{

				volume_[1UL << (NDIMS - i - 1)] = length_[i];

				inv_volume_[1UL << (NDIMS - i - 1)] = inv_length_[i];

			}
		}

		/**
		 *\verbatim
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
		 *\endverbatim
		 */

		volume_[0] = 1;
		//		volume_[1] /* 001 */= dx_[0];
		//		volume_[2] /* 010 */= dx_[1];
		//		volume_[4] /* 100 */= dx_[2];

		volume_[3] /* 011 */= volume_[1] * volume_[2];
		volume_[5] /* 101 */= volume_[4] * volume_[1];
		volume_[6] /* 110 */= volume_[2] * volume_[4];

		volume_[7] /* 111 */= volume_[1] * volume_[2] * volume_[4];

		inv_volume_[0] = 1;
		//		inv_volume_[1] /* 001 */= inv_dx_[0];
		//		inv_volume_[2] /* 010 */= inv_dx_[1];
		//		inv_volume_[4] /* 100 */= inv_dx_[2];

		inv_volume_[3] /* 011 */= inv_volume_[1] * inv_volume_[2];
		inv_volume_[5] /* 101 */= inv_volume_[4] * inv_volume_[1];
		inv_volume_[6] /* 110 */= inv_volume_[2] * inv_volume_[4];

		inv_volume_[7] /* 111 */= inv_volume_[1] * inv_volume_[2] * inv_volume_[4];

	}

	Real HodgeStarVolumeScale(compact_index_type s) const
	{
		return 1.0;
	}

	Real CellVolume(compact_index_type s) const
	{
		return topology_type::CellVolume(s) * volume_[1] * volume_[2] * volume_[4] * get_coordinates(s)[RAxis];
	}

	scalar_type Volume(compact_index_type s) const
	{
		unsigned int n = topology_type::NodeId(s);
		return topology_type::Volume(s) * volume_[n]
		        * (((n & (1UL << (NDIMS - PhiAxis - 1))) > 0) ? get_coordinates(s)[RAxis] : 1.0);
	}

	scalar_type InvVolume(compact_index_type s) const
	{
		unsigned int n = topology_type::NodeId(s);
		return topology_type::InvVolume(s) * inv_volume_[n]
		        / (((n & (1UL << (NDIMS - PhiAxis - 1))) > 0) ? get_coordinates(s)[RAxis] : 1.0);
	}

	scalar_type DualVolume(compact_index_type s) const
	{
		unsigned int n = topology_type::NodeId(topology_type::Dual(s));
		return topology_type::DualVolume(s) * volume_[n]
		        * (((n & (1UL << (NDIMS - PhiAxis - 1))) > 0) ? get_coordinates(s)[RAxis] : 1.0);
	}
	scalar_type InvDualVolume(compact_index_type s) const
	{
		unsigned int n = topology_type::NodeId(topology_type::Dual(s));
		return topology_type::InvDualVolume(s) * inv_volume_[n]
		        / (((n & (1UL << (NDIMS - PhiAxis - 1))) > 0) ? get_coordinates(s)[RAxis] : 1.0);
	}
	//! @}
}
;

}  // namespace simpla

#endif /* GEOMETRY_CYLINDRICAL_H_ */
