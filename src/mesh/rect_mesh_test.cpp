/*
 * rect_mesh_test.cpp
 *
 *  Created on: 2014年3月7日
 *      Author: salmon
 */

#include <gtest/gtest.h>

#include "../fetl/fetl.h"
#include "../io/data_stream.h"
#include "../utilities/log.h"

#include "octree_forest.h"

#define DEF_MESH RectMesh<>

using namespace simpla;

template<typename TI>
class TestMesh: public testing::Test
{
protected:
	virtual void SetUp()
	{
		Logger::Verbose(10);

		nTuple<3, size_t> dims = { 8, 9, 1 };
		mesh.SetDimensions(dims);

	}
public:
	typedef OcForest mesh_type;
	static constexpr int IForm = TI::value;
	typedef typename OcForest::index_type index_type;
	mesh_type mesh;

};
typedef testing::Types<Int2Type<VERTEX>, Int2Type<EDGE>, Int2Type<FACE>, Int2Type<VOLUME> > FormList;

TYPED_TEST_CASE(TestMesh, FormList);

TYPED_TEST(TestMesh, traversal){
{
	size_t count = 0;

	CHECK( TestFixture::mesh.GetDimensions());

	CHECK(TestFixture::mesh.GetNumOfElements(TestFixture::IForm ));

	auto s=*(TestFixture::mesh.begin(TestFixture::IForm ));

	s=*(TestFixture::mesh.begin(TestFixture::IForm ));

	TestFixture::mesh.template Traversal < TestFixture::IForm > (
			[& ](typename TestFixture::index_type s )
			{	++count;}
	);


	EXPECT_EQ(count,TestFixture:: mesh.GetNumOfElements( TestFixture::IForm));
}}