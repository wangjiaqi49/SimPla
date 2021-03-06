/*
 * geometry_test_Cylindrical.cpp
 *
 *  created on: 2014-6-27
 *      Author: salmon
 */

#include <gtest/gtest.h>

#include "../mesh/uniform_array.h"
#include "../mesh/geometry_cylindrical.h"

#define GEOMETRY  CylindricalGeometry<UniformArray>

#include "geometry_test.h"

using namespace simpla;

INSTANTIATE_TEST_CASE_P(CylindricalGeometry, TestGeometry,

testing::Combine(

testing::Values(nTuple<3, Real>( { 1.0, 0.0, 0.0, }) //
        , nTuple<3, Real>( { 1.0, 1.0, 1.0 })    //
        ),

testing::Values(
//
        nTuple<3, Real>( { 2.0, 0.0, 0.0 }), //
        nTuple<3, Real>( { 2.0, 0.0, 2.0 }), //
        nTuple<3, Real>( { 4.0, 2.0, 0.0 }), //
        nTuple<3, Real>( { 3.0, 3.0, 2.0 })  //

                ),

testing::Values(

nTuple<3, size_t>( { 10, 2, 1 }) //
        , nTuple<3, size_t>( { 11, 1, 1 }) //
        , nTuple<3, size_t>( { 17, 1, 17 }) //
        , nTuple<3, size_t>( { 13, 16, 10 })   //

        )));

