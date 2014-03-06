/*
 * octree_forest.h
 *
 *  Created on: 2014年2月21日
 *      Author: salmon
 */

#ifndef OCTREE_FOREST_H_
#define OCTREE_FOREST_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <thread>

#include "../fetl/ntuple.h"
#include "../fetl/primitives.h"
#include "../utilities/type_utilites.h"
#include "../utilities/memory_pool.h"

namespace simpla
{

struct OcForest
{

	typedef OcForest this_type;
	static constexpr int MAX_NUM_VERTEX_PER_CEL = 8;
	static constexpr int NUM_OF_DIMS = 3;

	typedef unsigned long size_type;
	typedef unsigned long compact_index_type;

	/**
	 * 	Thanks my wife Dr. CHEN Xiang Lan, for her advice on  these bitwise operation
	 * 	               m            m             m
	 * 	|--------|------------|--------------|-------------|
	 * 	               I              J             K
	 */
	//!< signed long is 63bit, unsigned long is 64 bit, add a sign bit
	static constexpr unsigned int FULL_DIGITS = std::numeric_limits<compact_index_type>::digits;

	static constexpr unsigned int MAX_TREE_HEIGHT = 4;

	static constexpr unsigned int INDEX_DIGITS = (FULL_DIGITS - CountBits<MAX_TREE_HEIGHT>::n) / 3;

	static constexpr unsigned int DIGITS_HEAD = FULL_DIGITS - INDEX_DIGITS * 3;

	static constexpr size_type INDEX_MAX = static_cast<size_type>(((1L) << (INDEX_DIGITS)) - 1);

	static constexpr size_type INDEX_MIN = 0;

	static constexpr double dh = 1.0 / static_cast<double>(INDEX_MAX + 1);

	//***************************************************************************************************

	nTuple<3, unsigned int> index_digits_ =
	{ INDEX_DIGITS - MAX_TREE_HEIGHT, INDEX_DIGITS - MAX_TREE_HEIGHT, INDEX_DIGITS - MAX_TREE_HEIGHT };

	compact_index_type _MI = 0UL;
	compact_index_type _MJ = 0UL;
	compact_index_type _MK = 0UL;
	compact_index_type _MA = _MI | _MJ | _MK;

	OcForest()
	{
	}
	template<typename TDict>
	OcForest(TDict const & dict)
	{

	}
	OcForest(nTuple<3, size_type> const & d)
	{
		SetDimensions(d);
	}

	~OcForest()
	{
	}

	this_type & operator=(const this_type&) = delete;

	void swap(OcForest & rhs)
	{
		std::swap(index_digits_, rhs.index_digits_);
		std::swap(_MI, rhs._MI);
		std::swap(_MJ, rhs._MJ);
		std::swap(_MK, rhs._MK);
		std::swap(_MA, rhs._MA);
	}

	template<typename TI>
	void SetDimensions(TI const &d)
	{
		index_digits_[0] = count_bits(d[0]);
		index_digits_[1] = count_bits(d[1]);
		index_digits_[2] = count_bits(d[2]);
		_MI = _C(index_type(
		{ 0, 1U << (INDEX_DIGITS - index_digits_[0]), 0, 0 }));
		_MJ = _C(index_type(
		{ 0, 0, 1U << (INDEX_DIGITS - index_digits_[1]), 0 }));
		_MK = _C(index_type(
		{ 0, 0, 0, 1U << (INDEX_DIGITS - index_digits_[2]) }));
		_MA = _MI | _MJ | _MK;

	}
	//***************************************************************************************************

	struct index_type
	{
		size_type H :DIGITS_HEAD;
		size_type I :INDEX_DIGITS;
		size_type J :INDEX_DIGITS;
		size_type K :INDEX_DIGITS;

#define DEF_OP(_OP_)                                                   \
		inline index_type operator _OP_##=(index_type const &r)        \
		{                                                              \
			H = std::max(H, r.H);                                      \
			I _OP_##= r.I;                                             \
			J _OP_##= r.J;                                             \
			K _OP_##= r.K;                                             \
			return *this;                                              \
		}                                                              \
        inline index_type operator _OP_ (index_type const &r)const     \
		{                                                              \
			index_type t;                                              \
			t.H = std::max(H, r.H);                                    \
			t.I = I _OP_ r.I;                                          \
			t.J = J _OP_ r.J;                                          \
			t.K = K _OP_ r.K;                                          \
			return t;                                                  \
		}                                                              \
		inline index_type operator _OP_##=(compact_index_type  r )     \
		{    this->operator _OP_##=(_C(r));  return *this;      }      \
		inline index_type operator _OP_ (compact_index_type  r)const   \
		{   return std::move(this->operator _OP_ (_C(r)));    }

		DEF_OP(+)
		DEF_OP(-)
		DEF_OP(^)
		DEF_OP(&)
		DEF_OP(|)
#undef DEF_OP

		bool operator==(index_type const & rhs) const
		{
			return _C(*this) == _C(rhs);
		}

	};

	nTuple<NUM_OF_DIMS, size_type> strides =
	{ 0, 0, 0 };

	template<int IFORM>
	inline size_type Hash(index_type s) const
	{

		return (

		(s.I >> (INDEX_DIGITS - index_digits_[0] - s.H + 1)) * strides[0] +

		(s.J >> (INDEX_DIGITS - index_digits_[1] - s.H + 1)) * strides[1] +

		(s.K >> (INDEX_DIGITS - index_digits_[2] - s.H + 1)) * strides[2]

		);

	}

	template<int IFORM>
	struct iterator
	{

		static constexpr int IForm = IFORM;

		OcForest const & tree;

		index_type s_;

		iterator(OcForest const & m, index_type s = _C(0UL)) :
				tree(m), s_(s)
		{
		}

		~iterator()
		{
		}

		bool operator==(iterator const & rhs) const
		{
			return s_ == rhs.s_;
		}

		bool operator!=(iterator const & rhs) const
		{
			return !(this->operator==(rhs));
		}

		index_type const & operator*() const
		{
			return s_;
		}

		size_type Hash() const
		{
			return tree.Hash<IForm>(s_);
		}

		iterator & operator ++()
		{
			s_ = tree.Next<IForm>(s_);
			return *this;
		}

	};

	template<int IFORM> iterator<IFORM> begin(int total = 1, int sub = 0) const
	{
		auto dims_ = GetDimensions();

		index_type s =
		{ 0, (dims_[0] * (sub) / total - 1) << (INDEX_DIGITS - index_digits_[0]), 0, 0 };

		return iterator<IFORM>(*this, s);
	}
	template<int IFORM> iterator<IFORM> end(int total = 1, int sub = 0) const
	{

		auto dims_ = GetDimensions();

		index_type s =
		{

		0,

		(dims_[0] * (sub + 1) / total - 1) << (INDEX_DIGITS - index_digits_[0]),

		dims_[1] << (INDEX_DIGITS - index_digits_[1]),

		dims_[2] << (INDEX_DIGITS - index_digits_[2])

		};
		return iterator<IFORM>(*this, s);
	}

	template<int IForm>
	index_type Next(index_type s) const
	{
		if (IForm == VERTEX || IForm == VOLUME || (IForm == EDGE && _N(s) == 2) || (IForm == FACE && _N(_I(s)) == 2))
		{
			s.K += 1U << (INDEX_DIGITS - index_digits_[2]);
			s.J += ((s.K >> (INDEX_DIGITS - index_digits_[2])) == 0) ? 1U << (INDEX_DIGITS - index_digits_[1]) : 0;
			s.I += ((s.J >> (INDEX_DIGITS - index_digits_[1])) == 0) ? 1U << (INDEX_DIGITS - index_digits_[0]) : 0;
		}
		else
		{
			s = _C(_R(s));
		}

		return s;
	}

	//***************************************************************************************************
	//  Traversal
	template<int IFORM, typename TL, typename TR>
	void Assign(Field<this_type, IFORM, TL> * f, Field<this_type, IFORM, TR> const & rhs) const
	{
		const unsigned int num_threads = std::thread::hardware_concurrency();

		std::vector<std::thread> threads;

		for (unsigned int thread_id = 0; thread_id < num_threads; ++thread_id)
		{
			auto ib = this->begin<IFORM>(num_threads, thread_id);
			auto ie = this->end<IFORM>(num_threads, thread_id);

			threads.emplace_back(
					std::thread([ib,ie](Field<this_type, IFORM, TL> * f2, Field<this_type, IFORM, TR> const & r )
					{
						for (auto it =ib; it != ie; ++it)
						{
							f2[*it]=r[*it];
						}

					}, f, std::forward<Field<this_type, IFORM, typename TR> const &>(rhs)

					));
		}

		for (auto & t : threads)
		{
			t.join();
		}
	}

	template<int IFORM, typename ... Args>
	void ParallelTraversal(std::function<void(index_type, Args ...)> fun, Args ...args) const
	{
		const unsigned int num_threads = std::thread::hardware_concurrency();

		std::vector<std::thread> threads;

		for (unsigned int thread_id = 0; thread_id < num_threads; ++thread_id)
		{
			auto ib = this->begin<IFORM>(num_threads, thread_id);
			auto ie = this->end<IFORM>(num_threads, thread_id);

			threads.emplace_back(

			std::thread(

			[ib,ie](std::function<void(index_type, Args ...)> fun2, Args ... args2 )
			{
				for (auto it =ib; it != ie; ++it)
				{
					fun2(*it,args2...);
				}

			}, fun, std::forward<Args >(args)...

			));
		}

		for (auto & t : threads)
		{
			t.join();
		}
	}

	template<int IFORM, typename ...Args>
	void Traversal(std::function<void(index_type, Args const & ...)> const &fun, Args const & ...args) const
	{
		for (auto it = this->begin<IFORM>(), ie = this->end<IFORM>(); it != ie; ++it)
		{
			fun(*it, std::forward<Args const &>(args)...);
		}
	}
	template<int IFORM>
	void Traversal(std::function<void(index_type)> const &fun) const
	{
		for (auto it = this->begin<IFORM>(), ie = this->end<IFORM>(); it != ie; ++it)
		{
			fun(*it);
		}
	}
	//***************************************************************************************************

	nTuple<3, size_type> GetDimensions() const
	{
		return nTuple<3, size_type>(
		{ 1U << index_digits_[0], 1U << index_digits_[1], 1U << index_digits_[2] });

	}
	nTuple<3, Real> GetDx() const
	{
		return nTuple<3, Real>(
		{

		static_cast<Real>(1U << (INDEX_DIGITS - index_digits_[0])) * dh,

		static_cast<Real>(1U << (INDEX_DIGITS - index_digits_[1])) * dh,

		static_cast<Real>(1U << (INDEX_DIGITS - index_digits_[2])) * dh });
	}

	inline index_type GetIndex(nTuple<3, Real> const & x, unsigned int H = 0) const
	{
		index_type res;

		ASSERT(0 <= x[0] && x[0] <= 1.0);

		ASSERT(0 <= x[1] && x[1] <= 1.0);

		ASSERT(0 <= x[2] && x[2] <= 1.0);

		res.H = H;

		res.I = static_cast<size_type>(std::floor(x[0] * static_cast<Real>(INDEX_MAX + 1)))
				& ((~0UL) << (INDEX_DIGITS - index_digits_[0] - H));

		res.J = static_cast<size_type>(std::floor(x[1] * static_cast<Real>(INDEX_MAX + 1)))
				& ((~0UL) << (INDEX_DIGITS - index_digits_[1] - H));

		res.K = static_cast<size_type>(std::floor(x[2] * static_cast<Real>(INDEX_MAX + 1)))
				& ((~0UL) << (INDEX_DIGITS - index_digits_[2] - H));

		return std::move(res);
	}

	inline nTuple<3, Real> GetCoordinates(index_type const & s) const
	{

		return nTuple<3, Real>(
		{

		static_cast<Real>(s.I) * dh,

		static_cast<Real>(s.J) * dh,

		static_cast<Real>(s.K) * dh

		});

	}

	//***************************************************************************************************
	//* Auxiliary functions
	//***************************************************************************************************

	static compact_index_type &_C(index_type &s)
	{
		return *reinterpret_cast<compact_index_type *>(&s);
	}

	static compact_index_type const &_C(index_type const &s)
	{
		return *reinterpret_cast<compact_index_type const*>(&s);
	}

	static index_type &_C(compact_index_type &s)
	{
		return *reinterpret_cast<index_type *>(&s);
	}

	static index_type const &_C(compact_index_type const &s)
	{
		return *reinterpret_cast<index_type const*>(&s);
	}
	/**
	 *  rotate vector direction  mask
	 *  (1/2,0,0) => (0,1/2,0) or   (1/2,1/2,0) => (0,1/2,1/2)
	 * @param s
	 * @return
	 */
	compact_index_type _R(compact_index_type s) const
	{
		s >>= (_C(s).H + 1);
		return (_MJ & (s & _MI)) | (_MK & (s & _MJ)) | (_MI & (s & _MK));
	}

	compact_index_type _R(index_type s) const
	{
		return std::move(_R(_C(s)));
	}

	/**
	 *  rotate vector direction  mask
	 *  (1/2,0,0) => (0,0,1/2) or   (1/2,1/2,0) => (1/2,0,1/2)
	 * @param s
	 * @return
	 */
	compact_index_type _RR(compact_index_type s) const
	{
		s >>= (_C(s).H + 1);
		return (_MK & (s & _MI)) | (_MI & (s & _MJ)) | (_MJ & (s & _MK));
	}

	compact_index_type _RR(index_type s) const
	{
		return std::move(_RR(_C(s)));
	}

	compact_index_type _I(compact_index_type s) const
	{
		return _MA & (~((s >> (_C(s).H + 1)) & _MA));
	}

	compact_index_type _I(index_type s) const
	{
		return std::move(_I(_C(s)));
	}

	//! get the direction of vector(edge) 0=>x 1=>y 2=>z
	size_type _N(index_type s) const
	{
		return ((s.J >> (INDEX_DIGITS - index_digits_[1] - s.H - 1)) & 1UL) |

		((s.K >> (INDEX_DIGITS - index_digits_[1] - s.H - 2)) & 2UL);
	}
	size_type _N(compact_index_type s) const
	{
		return std::move(_N(_C(s)));
	}

	template<int I>
	inline int GetAdjacentCells(Int2Type<I>, Int2Type<I>, index_type s, index_type *v) const
	{
		v[0] = s;
		return 1;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VERTEX>, index_type s, index_type *v) const
	{
		v[0] = s + s & (_MA >> (s.H + 1));
		v[1] = s - s & (_MA >> (s.H + 1));
		return 2;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VERTEX>, index_type s, index_type *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   2---------------*
		 *        |  /|              /|
		 *          / |             / |
		 *         /  |            /  |
		 *        3---|-----------*   |
		 *        | m |           |   |
		 *        |   1-----------|---*
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 *        0---------------*---> x
		 *
		 *
		 */

		auto di = (_R(_I(s)) >> (s.H + 1));
		auto dj = (_RR(_I(s)) >> (s.H + 1));

		v[0] = s - di - dj;
		v[1] = s - di - dj;
		v[2] = s + di + dj;
		v[3] = s + di + dj;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<VERTEX>, index_type const &s, index_type *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          / |             / |
		 *         /  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 *        0---------------1   ---> x
		 *
		 *
		 */
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = ((s - di) - dj) - dk;
		v[1] = ((s - di) - dj) + dk;
		v[2] = ((s - di) + dj) - dk;
		v[3] = ((s - di) + dj) + dk;

		v[4] = ((s + di) - dj) - dk;
		v[5] = ((s + di) - dj) + dk;
		v[6] = ((s + di) + dj) - dk;
		v[7] = ((s + di) + dj) + dk;

		return 8;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<EDGE>, index_type s, index_type *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          2 |             / |
		 *         /  1            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        3  /            |  /
		 *        | 0             | /
		 *        |/              |/
		 *        0------E0-------1   ---> x
		 *
		 *
		 */

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = s + di;
		v[1] = s - di;

		v[2] = s + dj;
		v[3] = s - dj;

		v[4] = s + dk;
		v[5] = s - dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<EDGE>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          2 |             / |
		 *         /  1            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        3  /            |  /
		 *        | 0             | /
		 *        |/              |/
		 *        0---------------1   ---> x
		 *
		 *
		 */
		auto d1 = (_R(_I(s)) >> (s.H + 1));
		auto d2 = (_RR(_I(s)) >> (s.H + 1));
		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<EDGE>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6------10-------7
		 *        |  /|              /|
		 *         11 |             9 |
		 *         /  7            /  6
		 *        4---|---8-------5   |
		 *        |   |           |   |
		 *        |   2-------2---|---3
		 *        4  /            5  /
		 *        | 3             | 1
		 *        |/              |/
		 *        0-------0-------1   ---> x
		 *
		 *
		 */
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = (s + di) + dj;
		v[1] = (s + di) - dj;
		v[2] = (s - di) + dj;
		v[3] = (s - di) - dj;

		v[4] = (s + dk) + dj;
		v[5] = (s + dk) - dj;
		v[6] = (s - dk) + dj;
		v[7] = (s - dk) - dj;

		v[8] = (s + di) + dk;
		v[9] = (s + di) - dk;
		v[10] = (s - di) + dk;
		v[11] = (s - di) - dk;

		return 12;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<FACE>, index_type s, index_type *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        | 0 2-----------|---3
		 *        |  /            |  /
		 *   11   | /      8      | /
		 *      3 |/              |/
		 * -------0---------------1   ---> x
		 *       /| 1
		 *10    / |     9
		 *     /  |
		 *      2 |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = (s + di) + dj;
		v[1] = (s + di) - dj;
		v[2] = (s - di) + dj;
		v[3] = (s - di) - dj;

		v[4] = (s + dk) + dj;
		v[5] = (s + dk) - dj;
		v[6] = (s - dk) + dj;
		v[7] = (s - dk) - dj;

		v[8] = (s + di) + dk;
		v[9] = (s + di) - dk;
		v[10] = (s - di) + dk;
		v[11] = (s - di) - dk;

		return 12;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<FACE>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /  0         |  /
		 *        | /      1      | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *      / |   3
		 *     /  |       2
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto d1 = (_R(s) >> (s.H + 1));
		auto d2 = (_RR(s) >> (s.H + 1));

		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<FACE>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^    /
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |    5        / |
		 *        |/  |     1      /  |
		 *        4---|-----------5   |
		 *        | 0 |           | 2 |
		 *        |   2-----------|---3
		 *        |  /    3       |  /
		 *        | /       4     | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *
		 */

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = s - di;
		v[1] = s + di;

		v[2] = s - di;
		v[3] = s + dj;

		v[4] = s - dk;
		v[5] = s + dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<VOLUME>, index_type s, index_type *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *   3    |   |    0      |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *  3    /|       1
		 *      / |
		 *     /  |
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = ((s - di) - dj) - dk;
		v[1] = ((s - di) - dj) + dk;
		v[2] = ((s - di) + dj) - dk;
		v[3] = ((s - di) + dj) + dk;

		v[4] = ((s + di) - dj) - dk;
		v[5] = ((s + di) - dj) + dk;
		v[6] = ((s + di) + dj) - dk;
		v[7] = ((s + di) + dj) + dk;

		return 8;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VOLUME>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /  0         |  /
		 *        | /      1      | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *      / |   3
		 *     /  |       2
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto d1 = (_R(s) >> (s.H + 1));
		auto d2 = (_RR(s) >> (s.H + 1));

		v[0] = s - d1 - d2;
		v[1] = s + d1 - d2;
		v[2] = s - d1 + d2;
		v[3] = s + d1 + d2;
		return 4;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VOLUME>, index_type s, index_type *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^    /
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        | 0 |           |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *
		 */

		auto d = (_I(s) >> (s.H + 1));
		v[0] = s + d;
		v[1] = s - d;

		return 2;
	}

}
;

}
// namespace simpla

#endif /* OCTREE_FOREST_H_ */