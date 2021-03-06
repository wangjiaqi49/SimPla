/*
 * read_geqdsk.cpp
 *
 *  created on: 2013-11-29
 *      Author: salmon
 */

#include "geqdsk.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <map>
#include <memory>
#include <utility>

#include "../utilities/log.h"
#include "../utilities/pretty_stream.h"
#include "../utilities/ntuple.h"
#include "../physics/constants.h"
#include "../io/data_stream.h"
#include "../numeric/find_root.h"
#include "../numeric/pointinpolygon.h"

namespace simpla
{

void GEqdsk::load(std::string const &fname)
{

	std::ifstream inFileStream_(fname);

	if (!inFileStream_.is_open())
	{
		RUNTIME_ERROR("File " + fname + " is not opend!");
		return;
	}

	nTuple<size_t, ndims> dims;
	coordinates_type rzmin;
	coordinates_type rzmax;

	LOGGER << "Load GFile : " << fname;

	int nw; //Number of horizontal R grid points
	int nh; //Number of vertical Z grid points
	double simag; // Poloidal flux at magnetic axis in Weber / rad
	double sibry; // Poloidal flux at the plasma boundary in Weber / rad
	int idum;
	double xdum;

	char str_buff[50];

	inFileStream_.get(str_buff, 48);

	desc = std::string(str_buff);

	inFileStream_ >> std::setw(4) >> idum >> nw >> nh;

	inFileStream_ >> std::setw(16)

	>> rdim >> zdim >> rcenter >> rleft >> zmid

	>> rmaxis >> zmaxis >> simag >> sibry >> bcenter

	>> current >> simag >> xdum >> rmaxis >> xdum

	>> zmaxis >> xdum >> sibry >> xdum >> xdum;

	rzmin[RAxis] = rleft;
	rzmax[RAxis] = rleft + rdim;

	rzmin[ZAxis] = zmid - zdim / 2;
	rzmax[ZAxis] = zmid + zdim / 2;

	rzmin[PhiAxis] = 0;
	rzmax[PhiAxis] = 0;

	dims[RAxis] = nw;
	dims[ZAxis] = nh;
	dims[PhiAxis] = 0;

	geometry_type::dimensions(dims);
	geometry_type::extents(rzmin, rzmax);

	size_t phi_axis = PhiAxis;

	inter2d_type(dims, rzmin, rzmax, phi_axis).swap(psirz_);

#define INPUT_VALUE(_NAME_)                                                            \
	for (int s = 0; s < nw; ++s)                                              \
	{                                                                                  \
		double y;                                                                  \
		inFileStream_ >> std::setw(16) >> y;                                           \
		profile_[ _NAME_ ].data().emplace(                                                         \
	      static_cast<double>(s)                                              \
	          /static_cast<double>(nw-1), y );                               \
	}                                                                                  \

	INPUT_VALUE("fpol");
	INPUT_VALUE("pres");
	INPUT_VALUE("ffprim");
	INPUT_VALUE("pprim");

	for (int j = 0; j < nh; ++j)
		for (int i = 0; i < nw; ++i)
		{
			double v;
			inFileStream_ >> std::setw(16) >> v;
			psirz_[i + j * nw] = (v - simag) / (sibry - simag); // Normalize Poloidal flux
		}

	INPUT_VALUE("qpsi");

#undef INPUT_VALUE

	size_t nbbbs, limitr;
	inFileStream_ >> std::setw(5) >> nbbbs >> limitr;

	std::vector<nTuple<double, 2>> rzbbb(nbbbs);
	std::vector<nTuple<double, 2>> rzlim(limitr);

	inFileStream_ >> std::setw(16) >> rzbbb;
	inFileStream_ >> std::setw(16) >> rzlim;

	rzbbb_.resize(nbbbs);
	rzlim_.resize(limitr);

	for (size_t s = 0; s < nbbbs; ++s)
	{
		rzbbb_[s][RAxis] = rzbbb[s][0];
		rzbbb_[s][ZAxis] = rzbbb[s][1];
		rzbbb_[s][PhiAxis] = 0;
	}

	for (size_t s = 0; s < limitr; ++s)
	{
		rzlim_[s][RAxis] = rzlim[s][0];
		rzlim_[s][ZAxis] = rzlim[s][1];
		rzlim_[s][PhiAxis] = 0;
	}
	load_profile(fname + "_profiles.txt");

}
void GEqdsk::load_profile(std::string const &fname)
{
	LOGGER << "Load GFile Profiles: " << fname;
	std::ifstream inFileStream_(fname);

	if (!inFileStream_.is_open())
	{
		RUNTIME_ERROR("File " + fname + " is not opend!");
	}

	std::string line;

	std::getline(inFileStream_, line);

	std::vector<std::string> names;
	{
		std::stringstream lineStream(line);

		while (lineStream)
		{
			std::string t;
			lineStream >> t;
			if (t != "")
				names.push_back(t);
		};
	}

	while (inFileStream_)
	{
		auto it = names.begin();
		auto ie = names.end();
		double psi;
		inFileStream_ >> psi; 		/// \note assume first row is psi
		*it = psi;

		for (++it; it != ie; ++it)
		{
			double value;
			inFileStream_ >> value;
			profile_[*it].data().emplace(psi, value);

		}
	}
	std::string profile_list = "psi,B";

	for (auto const & item : profile_)
	{
		profile_list += " , " + item.first;
	}

	LOGGER << "GFile is ready! Profile={" << profile_list << "}" << std::endl;

	is_ready_ = true;
}

std::string GEqdsk::save(std::string const & path) const
{
	if (!is_ready())
	{
		return "";
	}

	GLOBAL_DATA_STREAM.cd(path);

	auto dd = geometry_type::dimensions();

	size_t d[2] =
	{ dd[RAxis], dd[ZAxis] };

	LOGGER << simpla::save("psi", psirz_.data(), 2, nullptr, d) << std::endl;

	LOGGER << simpla::save("rzbbb", rzbbb_) << std::endl;

	LOGGER << simpla::save("rzlim", rzlim_) << std::endl;

	for (auto const & p : profile_)
	{
		LOGGER << simpla::save(p.first, p.second.data()) << std::endl;
	}
	return path;
}
std::ostream & GEqdsk::print(std::ostream & os)
{
	std::cout << "--" << desc << std::endl;

//	std::cout << "nw" << "\t= " << nw
//			<< "\t--  Number of horizontal R grid  points" << std::endl;
//
//	std::cout << "nh" << "\t= " << nh << "\t-- Number of vertical Z grid points"
//			<< std::endl;
//
//	std::cout << "rdim" << "\t= " << rdim
//			<< "\t-- Horizontal dimension in meter of computational box                   "
//			<< std::endl;
//
//	std::cout << "zdim" << "\t= " << zdim
//			<< "\t-- Vertical dimension in meter of computational box                   "
//			<< std::endl;

	std::cout << "rcentr" << "\t= " << rcenter
			<< "\t--                                                                    "
			<< std::endl;

//	std::cout << "rleft" << "\t= " << rleft
//			<< "\t-- Minimum R in meter of rectangular computational box                "
//			<< std::endl;
//
//	std::cout << "zmid" << "\t= " << zmid
//			<< "\t-- Z of center of computational box in meter                          "
//			<< std::endl;

	std::cout << "rmaxis" << "\t= " << rmaxis
			<< "\t-- R of magnetic axis in meter                                        "
			<< std::endl;

	std::cout << "rmaxis" << "\t= " << zmaxis
			<< "\t-- Z of magnetic axis in meter                                        "
			<< std::endl;

//	std::cout << "simag" << "\t= " << simag
//			<< "\t-- poloidal flus ax magnetic axis in Weber / rad                      "
//			<< std::endl;
//
//	std::cout << "sibry" << "\t= " << sibry
//			<< "\t-- Poloidal flux at the plasma boundary in Weber / rad                "
//			<< std::endl;

	std::cout << "rcentr" << "\t= " << rcenter
			<< "\t-- R in meter of  vacuum toroidal magnetic field BCENTR               "
			<< std::endl;

	std::cout << "bcentr" << "\t= " << bcenter
			<< "\t-- Vacuum toroidal magnetic field in Tesla at RCENTR                  "
			<< std::endl;

	std::cout << "current" << "\t= " << current
			<< "\t-- Plasma current in Ampere                                          "
			<< std::endl;

//	std::cout << "fpol" << "\t= "
//			<< "\t-- Poloidal current function in m-T<< $F=RB_T$ on flux grid           "
//			<< std::endl << fpol_.data() << std::endl;
//
//	std::cout << "pres" << "\t= "
//			<< "\t-- Plasma pressure in $nt/m^2$ on uniform flux grid                   "
//			<< std::endl << pres_.data() << std::endl;
//
//	std::cout << "ffprim" << "\t= "
//			<< "\t-- $FF^\\prime(\\psi)$ in $(mT)^2/(Weber/rad)$ on uniform flux grid     "
//			<< std::endl << ffprim_.data() << std::endl;
//
//	std::cout << "pprim" << "\t= "
//			<< "\t-- $P^\\prime(\\psi)$ in $(nt/m^2)/(Weber/rad)$ on uniform flux grid    "
//			<< std::endl << pprim_.data() << std::endl;
//
//	std::cout << "psizr"
//			<< "\t-- Poloidal flus in Webber/rad on the rectangular grid points         "
//			<< std::endl << psirz_.data() << std::endl;
//
//	std::cout << "qpsi" << "\t= "
//			<< "\t-- q values on uniform flux grid from axis to boundary                "
//			<< std::endl << qpsi_.data() << std::endl;
//
//	std::cout << "nbbbs" << "\t= " << nbbbs
//			<< "\t-- Number of boundary points                                          "
//			<< std::endl;
//
//	std::cout << "limitr" << "\t= " << limitr
//			<< "\t-- Number of limiter points                                           "
//			<< std::endl;
//
//	std::cout << "rzbbbs" << "\t= "
//			<< "\t-- R of boundary points in meter                                      "
//			<< std::endl << rzbbb_ << std::endl;
//
//	std::cout << "rzlim" << "\t= "
//			<< "\t-- R of surrounding limiter contour in meter                          "
//			<< std::endl << rzlim_ << std::endl;

	return os;
}

bool GEqdsk::FluxSurface(double psi_j, size_t M, coordinates_type*res,
		size_t ToPhiAxis, double resolution)
{
	bool success = true;

	PointInPolygon boundary(rzbbb_, PhiAxis);

	nTuple<double, 3> center;

	center[PhiAxis] = 0;
	center[RAxis] = rcenter;
	center[ZAxis] = zmid;

	nTuple<double, 3> drz;

	drz[PhiAxis] = 0;

	std::function<double(nTuple<double, 3> const &)> fun =
			[this ](nTuple<double,3> const & x)->double
			{
				return this->psi(x);
			};

	for (int i = 0; i < M; ++i)
	{
		double theta = static_cast<double>(i) * TWOPI / static_cast<double>(M);

		drz[RAxis] = std::cos(theta);
		drz[ZAxis] = std::sin(theta);

		nTuple<double, 3> rmax;
		nTuple<double, 3> t;
		// FIXME ntuple<Expression> auto convert
		t = center + drz * std::sqrt(rdim * rdim + zdim * zdim) * 0.5;

		std::tie(success, rmax) = boundary.Intersection(center, t);

		if (!success)
		{
			RUNTIME_ERROR(
					"Illegal Geqdsk configuration: RZ-center is out of the boundary (rzbbb)!  ");
		}
		// FIXME ntuple<Expression> auto convert
		nTuple<double, 3> t2 = center + (rmax - center) * 0.1;
		std::tie(success, res[i]) = find_root(t2, rmax, fun, psi_j, resolution);

		if (!success)
		{
			WARNING << "Construct flux surface failed!" << "at theta = "
					<< theta << " psi = " << psi_j;
			break;
		}

	}

	return success;

}

bool GEqdsk::MapToFluxCoordiantes(std::vector<coordinates_type> const&surface,
		std::vector<coordinates_type> *res,
		std::function<double(double, double)> const & h, size_t PhiAxis)
{
	bool success = false;

	UNIMPLEMENT;

	return success;

}

}  // namespace simpla

