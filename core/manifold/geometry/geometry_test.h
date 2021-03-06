/*
 * geometry_test.h
 *
 *  created on: 2014-6-27
 *      Author: salmon
 */

#ifndef COORDINATES_TEST_H_
#define COORDINATES_TEST_H_

#include <gtest/gtest.h>

#include "../../utilities/pretty_stream.h"
#include "../../utilities/log.h"
#include "../../parallel/message_comm.h"

using namespace simpla;

#ifndef GEOMETRY
#include "../topology/structured.h"
#include "cartesian.h"

typedef CartesianCoordinates<StructuredMesh> TGeometry;
#else
typedef GEOMETRY TGeometry;
#endif

class TestGeometry: public testing::TestWithParam<
		std::tuple<typename TGeometry::coordinates_type,
				typename TGeometry::coordinates_type,
				nTuple<size_t, TGeometry::ndims> > >
{
protected:
	void SetUp()
	{
		LOGGER.set_stdout_visable_level(10);

		auto param = GetParam();

		xmin = std::get<0>(param);

		xmax = std::get<1>(param);

		dims = std::get<2>(param);

		geometry.dimensions(dims);
		geometry.extents(xmin, xmax);
		geometry.update();

		std::tie(xmin, xmax) = geometry.extents();

	}
public:
	typedef TGeometry geometry_type;
	typedef typename geometry_type::index_type index_type;
	typedef typename geometry_type::range_type range_type;
	typedef typename geometry_type::scalar_type scalar_type;
	typedef typename geometry_type::iterator iterator;
	typedef typename geometry_type::coordinates_type coordinates_type;

	static constexpr size_t ndims = geometry_type::ndims;

	geometry_type geometry;

	std::vector<size_t> iform_list = { VERTEX, EDGE, FACE, VOLUME };
	coordinates_type xmin, xmax;
	nTuple<index_type, geometry_type::ndims> dims;

	Real epsilon = EPSILON * 10;

	bool is_valid() const
	{
		size_t nd = 0;
		for (int i = 0; i < ndims; ++i)
		{
			if (dims[i] > 1 && (xmax[i] > xmin[i]))
				++nd;
		}

		return nd > 0;
	}

};

TEST_P(TestGeometry, Coordinates)
{

	if (!is_valid())
		return;

	auto extents = geometry.extents();
	coordinates_type x = 0.21235 * (std::get<1>(extents) - std::get<0>(extents))
			+ std::get<0>(extents);

	for (auto iform : iform_list)
	{
		auto idx = geometry.coordinates_global_to_local(x,
				geometry.get_first_node_shift(iform));
		auto y = geometry.coordinates_local_to_global(idx);
		EXPECT_LE( (abs(x[0]- y[0])),10000*EPSILON) << y[0] - x[0];
		EXPECT_LE( (abs(x[1]- y[1])),10000*EPSILON) << y[1] - x[1];
		EXPECT_LE( (abs(x[2]- y[2])),10000*EPSILON) << y[2] - x[2];

		auto s = std::get<0>(idx);
		EXPECT_EQ(iform, geometry.IForm(s));
		EXPECT_EQ(geometry.node_id(geometry.get_first_node_shift(iform)),
				geometry.node_id(s));
		EXPECT_EQ(
				geometry.component_number(geometry.get_first_node_shift(iform)),
				geometry.component_number(s));

		EXPECT_GE(3, dot(std::get<1>(idx), std::get<1>(idx)));

	}

	auto idx = geometry.coordinates_to_index(x);
	auto jdx = geometry.coordinates_to_index(
			geometry.index_to_coordinates(idx));

	auto y = geometry.index_to_coordinates(geometry.coordinates_to_index(x));
	for (int i = 0; i < ndims; ++i)
	{
		if (dims[i] <= 1)
			x[i] = 0;
	}

	EXPECT_GT(EPSILON, NSum(abs(x - y))) << x << y;

	EXPECT_EQ(idx, jdx);

}

TEST_P(TestGeometry, Volume)
{

	if (!is_valid())
		return;
//	auto extents = geometry.extents();
//	coordinates_type x = 0.21235 * (std::get<1>(extents) - std::get<0>(extents)) + std::get<0>(extents);

	for (auto iform : iform_list)
	{
		for (auto s : geometry.select(iform))
		{
			auto IX = geometry_type::DI(0, s);
			auto IY = geometry_type::DI(1, s);
			auto IZ = geometry_type::DI(2, s);

			ASSERT_DOUBLE_EQ(geometry.cell_volume(s),
					geometry.dual_volume(s) * geometry.volume(s));
			ASSERT_DOUBLE_EQ(1.0 / geometry.cell_volume(s),
					geometry.inv_dual_volume(s) * geometry.inv_volume(s));

			ASSERT_DOUBLE_EQ(1.0, geometry.inv_volume(s) * geometry.volume(s));
			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_dual_volume(s) * geometry.dual_volume(s));

			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_volume(s + IX) * geometry.volume(s + IX));
			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_dual_volume(s + IX)
							* geometry.dual_volume(s + IX));

			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_volume(s - IY) * geometry.volume(s - IY));
			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_dual_volume(s - IY)
							* geometry.dual_volume(s - IY));

			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_volume(s - IZ) * geometry.volume(s - IZ));
			ASSERT_DOUBLE_EQ(1.0,
					geometry.inv_dual_volume(s - IZ)
							* geometry.dual_volume(s - IZ));
		}
	}

	auto extents = geometry.extents();
	coordinates_type x = 0.21235 * (std::get<1>(extents) - std::get<0>(extents))
			+ std::get<0>(extents);
	auto idx = geometry.coordinates_global_to_local(x,
			geometry.get_first_node_shift(VERTEX));

	auto s = std::get<0>(idx);
	auto IX = geometry_type::DI(0, s) << 1;
	auto IY = geometry_type::DI(1, s) << 1;
	auto IZ = geometry_type::DI(2, s) << 1;

//	CHECK_BIT(s);
//	CHECK_BIT(IX);
//	CHECK(geometry.volume(s - IX));
//	CHECK(geometry.volume(s + IX));
//	CHECK(geometry.volume(s - IY));
//	CHECK(geometry.volume(s + IY));
//	CHECK(geometry.volume(s - IZ));
//	CHECK(geometry.volume(s + IZ));
}

TEST_P(TestGeometry,Coordinates_transform)
{

	if (!is_valid())
		return;
	nTuple<Real, 3> v = { 1.0, 2.0, 3.0 };
	auto extents = geometry.extents();
	coordinates_type x = 0.21235 * (std::get<1>(extents) - std::get<0>(extents))
			+ std::get<0>(extents);
	auto z = std::make_tuple(x, v);

	coordinates_type y = geometry.InvMapTo(geometry.MapTo(x));
	EXPECT_DOUBLE_EQ(x[0], y[0]);
	EXPECT_DOUBLE_EQ(x[1], y[1]);
	EXPECT_DOUBLE_EQ(x[2], y[2]);

	y = geometry.MapTo(geometry.InvMapTo(x));
	EXPECT_DOUBLE_EQ(x[0], y[0]);
	EXPECT_DOUBLE_EQ(x[1], y[1]);
	EXPECT_DOUBLE_EQ(x[2], y[2]);

	auto z1 = geometry.push_forward(geometry.pull_back(z));
	EXPECT_DOUBLE_EQ(std::get<0>(z)[0], std::get<0>(z1)[0]);
	EXPECT_DOUBLE_EQ(std::get<0>(z)[1], std::get<0>(z1)[1]);
	EXPECT_DOUBLE_EQ(std::get<0>(z)[2], std::get<0>(z1)[2]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[0], std::get<1>(z1)[0]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[1], std::get<1>(z1)[1]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[2], std::get<1>(z1)[2]);

	auto z2 = geometry.pull_back(geometry.push_forward(z));
	EXPECT_DOUBLE_EQ(std::get<0>(z)[0], std::get<0>(z2)[0]);
	EXPECT_DOUBLE_EQ(std::get<0>(z)[1], std::get<0>(z2)[1]);
	EXPECT_DOUBLE_EQ(std::get<0>(z)[2], std::get<0>(z2)[2]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[0], std::get<1>(z2)[0]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[1], std::get<1>(z2)[1]);
	EXPECT_DOUBLE_EQ(std::get<1>(z)[2], std::get<1>(z2)[2]);

}

#endif /* COORDINATES_TEST_H_ */
