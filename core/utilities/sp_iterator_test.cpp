/*
 * iterator_test.cpp
 *
 *  created on: 2014-6-15
 *      Author: salmon
 */

#include <gtest/gtest.h>
#include "log.h"
using namespace simpla;

class TestIterator: public testing::TestWithParam<nTuple<3, size_t> >
{
protected:
	virtual void SetUp()
	{
		LOGGER.set_stdout_visable_level(12);

		auto param = GetParam();

	}
public:

	nTuple<3, Real> xmin, xmax;

	nTuple<3, size_t> dims;

	std::string cfg_str;

	bool enable_sorting;

};

TEST_P(TestIterator,Add)
{

}
INSTANTIATE_TEST_CASE_P(FETL, TestIterator,

		testing::Values(
				nTuple<3, size_t>(
						{	12, 16, 10}) //
				// ,nTuple<3, size_t>( { 1, 1, 1 }) //
				//        , nTuple<3, size_t>( { 17, 1, 1 }) //
				//        , nTuple<3, size_t>( { 1, 17, 1 }) //
				//        , nTuple<3, size_t>( { 1, 1, 10 }) //
				//        , nTuple<3, size_t>( { 1, 10, 20 }) //
				//        , nTuple<3, size_t>( { 17, 1, 17 }) //
				//        , nTuple<3, size_t>( { 17, 17, 1 }) //
				//        , nTuple<3, size_t>( { 12, 16, 10 })

		)));
