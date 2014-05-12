/*
 * data_stream.h
 *
 *  Created on: 2013年12月11日
 *      Author: salmon
 *
 *
 * TODO: DataStream and DataSet need improvement!!!
 */

#ifndef DATA_STREAM_
#define DATA_STREAM_

//#include <H5Epublic.h>
//#include <H5Ipublic.h>
//#include <H5LTpublic.h>
//#include <H5public.h>
//#include <H5Tpublic.h>
#include <complex>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

extern "C"
{
#include "hdf5.h"
#include "hdf5_hl.h"

}

#include "../fetl/ntuple.h"
#include "../utilities/log.h"
#include "../utilities/singleton_holder.h"
#include "../utilities/utilities.h"
#include "../utilities/pretty_stream.h"

#ifdef USE_MPI_IO
#include "../parallel/message_comm.h"
#endif

namespace simpla
{

#define H5_ERROR( _FUN_ ) if((_FUN_)<0){ H5Eprint(H5E_DEFAULT, stderr);}

namespace _impl
{

HAS_STATIC_MEMBER_FUNCTION(DataTypeDesc);

template<typename T>
typename std::enable_if<has_static_member_function_DataTypeDesc<T>::value, hid_t>::type GetH5Type()
{
	hid_t res;
	H5_ERROR(res = H5LTtext_to_dtype(T::DataTypeDesc().c_str(), H5LT_DDL));
	return res;
}
template<typename T>
typename std::enable_if<!has_static_member_function_DataTypeDesc<T>::value, hid_t>::type GetH5Type()
{
	return H5T_OPAQUE;
}

}  // namespace _impl

template<typename T>
struct HDF5DataType
{
	hid_t type(...) const
	{
		return _impl::GetH5Type<T>();
	}
};

template<> struct HDF5DataType<int>
{
	hid_t type() const
	{
		return H5T_NATIVE_INT;
	}
};

template<> struct HDF5DataType<float>
{
	hid_t type() const
	{
		return H5T_NATIVE_FLOAT;
	}
};

template<> struct HDF5DataType<double>
{
	hid_t type() const
	{
		return H5T_NATIVE_DOUBLE;
	}
};

template<> struct HDF5DataType<long double>
{
	hid_t type() const
	{
		return H5T_NATIVE_LDOUBLE;
	}
};
template<typename T> struct HDF5DataType<std::complex<T>>
{
	hid_t type_;
	HDF5DataType()
	{
		type_ = H5Tcreate(H5T_COMPOUND, sizeof(std::complex<T>));
		H5Tinsert(type_, "r", 0, HDF5DataType<T>().type());
		H5Tinsert(type_, "i", sizeof(T), HDF5DataType<T>().type());
	}

	~ HDF5DataType()
	{
		H5Tclose(type_);
	}

	hid_t type() const
	{
		return type_;
	}
};

template<typename TL, typename TR>
struct HDF5DataType<std::pair<TL, TR> >
{
	typedef std::pair<TL, TR> value_type;
	hid_t type_;
	HDF5DataType()
	{
		type_ = H5Tcreate(H5T_COMPOUND, sizeof(value_type));
		H5Tinsert(type_, "first", offsetof(value_type, first), HDF5DataType<TL>().type());
		H5Tinsert(type_, "second", offsetof(value_type, second), HDF5DataType<TR>().type());
	}

	~ HDF5DataType()
	{
		H5Tclose(type_);
	}

	hid_t type() const
	{
		return type_;
	}
};

class DataStream
{
	std::string prefix_;
	int suffix_width_;

	std::string filename_;
	std::string grpname_;
	hid_t file_;
	hid_t group_;
	size_t LIGHT_DATA_LIMIT_;
	bool is_compact_storable_;
public:

	DataStream()
			: prefix_("simpla_unnamed"), filename_("unnamed"), grpname_(""),

			file_(-1), group_(-1),

			suffix_width_(4),

			LIGHT_DATA_LIMIT_(20),

			is_compact_storable_(false)

	{
		hid_t error_stack = H5Eget_current_stack();
		H5Eset_auto(error_stack, NULL, NULL);
	}

	~DataStream()
	{
		CloseFile();
	}
	void SetLightDatLimit(size_t s)
	{
		LIGHT_DATA_LIMIT_ = s;
	}
	size_t GetLightDatLimit() const
	{
		return LIGHT_DATA_LIMIT_;
	}

	void EnableCompactStorable()
	{
		is_compact_storable_ = true;
	}
	void DisableCompactStorable()
	{
		is_compact_storable_ = false;
	}

	bool CheckCompactStorable() const
	{
		return is_compact_storable_;
	}

	inline std::string GetCurrentPath() const
	{
		return filename_ + ":" + grpname_;
	}

	inline std::string GetPrefix() const
	{
		return prefix_;
	}

	inline void SetPrefix(const std::string& prefix)
	{
		prefix_ = prefix;
	}

	int GetSuffixWidth() const
	{
		return suffix_width_;
	}

	void SetSuffixWidth(int suffixWidth)
	{
		suffix_width_ = suffixWidth;
	}

	void OpenGroup(std::string const & gname);
	void OpenFile(std::string const &fname = "unnamed");
	void CloseGroup();
	void CloseFile();

	template<typename TV>
	std::string Write(TV const *v, std::string const &name, int rank, size_t const * global_dims,
	        size_t const * offset = nullptr, size_t const * local_dims = nullptr, size_t const * start = nullptr,
	        size_t const *counts = nullptr, size_t const * strides = nullptr, size_t const *blocks = nullptr) const
	{

		hsize_t global_dims_[rank + 1];
		hsize_t offset_[rank + 1];
		hsize_t local_dims_[rank + 1];
		hsize_t start_[rank + 1];
		hsize_t counts_[rank + 1];
		hsize_t strides_[rank + 1];
		hsize_t blocks_[rank + 1];

		if (global_dims != nullptr)
			std::copy(global_dims, global_dims + rank, global_dims_);
		if (offset != nullptr)
			std::copy(offset, offset + rank, offset_);
		if (local_dims != nullptr)
			std::copy(local_dims, local_dims + rank, local_dims_);
		if (start != nullptr)
			std::copy(start, start + rank, start_);
		if (counts != nullptr)
			std::copy(counts, counts + rank, counts_);
		if (strides != nullptr)
			std::copy(strides, strides + rank, strides_);
		if (blocks != nullptr)
			std::copy(blocks, blocks + rank, blocks_);

		if (is_nTuple<TV>::value)
		{
			if (global_dims != nullptr)
				global_dims_[rank] = nTupleTraits<TV>::NDIMS;
			if (local_dims != nullptr)
				local_dims_[rank] = nTupleTraits<TV>::NDIMS;
			if (counts != nullptr)
				counts_[rank] = nTupleTraits<TV>::NDIMS;
			if (blocks != nullptr)
				blocks_[rank] = nTupleTraits<TV>::NDIMS;
			++rank;
		}

		return WriteHDF5(reinterpret_cast<void const*>(v), name,

		HDF5DataType<typename nTupleTraits<TV>::value_type>().type(),

		rank,

		global_dims_,

		offset_,

		local_dims_,

		start_,

		counts_,

		strides_,

		blocks_);

	}

	std::string WriteHDF5(void const *v, std::string const &name, hid_t mdtype, int rank, hsize_t const *global_dims,
	        hsize_t const *offset, hsize_t const *local_dims, hsize_t const *start, hsize_t const *counts,
	        hsize_t const *strides, hsize_t const *blocks) const;

}
;

#define GLOBAL_DATA_STREAM  SingletonHolder<DataStream> ::instance()

template<typename TV, typename ...Args>
inline std::string Save(TV const *data, std::string const & name, Args const & ...args)
{
	return GLOBAL_DATA_STREAM.Write(data, name, std::forward<Args const &>(args)...);
}

template<typename TV, typename ... Args> inline std::string Save(std::shared_ptr<TV> const & d, Args const & ... args)
{
	return Save(d.get(), std::forward<Args const &>(args)...);
}

template<typename TV, int rank, typename TS> inline std::string Save(TV const* data, std::string const &name,
        nTuple<rank, TS> const & d)
{
	return Save(reinterpret_cast<void const *>(data), name, rank, &d[0]);
}

template<typename TV, typename ... Args> inline std::string Save(std::vector<TV>const & d, std::string const & name,
        Args const & ... args)
{
	size_t s = d.size();

	return Save(&d[0], name, 1, &s, std::forward<Args const &>(args)...);
}
template<typename TL, typename TR, typename ... Args> inline std::string Save(std::map<TL, TR>const & d,
        std::string const & name, Args const & ... args)
{
	std::vector<std::pair<TL, TR> > d_;
	for (auto const & p : d)
	{
		d_.emplace_back(p);
	}
	return Save(d_, name, std::forward<Args const &>(args)...);
}

template<typename TV, typename ... Args> inline std::string Save(std::map<TV, TV>const & d, std::string const & name,
        Args const & ... args)
{
	std::vector<nTuple<2, TV> > d_;
	for (auto const & p : d)
	{
		d_.emplace_back(nTuple<2, TV>( { p.first, p.second }));
	}
	return Save(d_, name, std::forward<Args const &>(args)...);
}

#define SAVE(_F_) simpla::Save(_F_,__STRING(_F_)  )
#ifndef NDEBUG
#	define DEBUG_SAVE(_F_) simpla::Save(_F_,__STRING(_F_) )
#else
#   define DEBUG_SAVE(_F_) ""
#endif
}
// namespace simpla

#endif /* DATA_STREAM_ */
