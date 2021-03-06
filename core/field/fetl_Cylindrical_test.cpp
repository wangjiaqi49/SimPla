/*
 * fetl_test3_Cylindrical.cpp
 *
 *  created on: 2014-6-23
 *      Author: salmon
 */

#include "../mesh/mesh_rectangle.h"
#include "../mesh/uniform_array.h"
#include "../mesh/geometry_cylindrical.h"
using namespace simpla;
#define TMESH Mesh<CylindricalGeometry<UniformArray>>

#include "fetl_test.h"
#include "fetl_test1.h"
#include "fetl_test2.h"
#include "fetl_test3.h"
#include "fetl_test4.h"

INSTANTIATE_TEST_CASE_P(FETLCylindrical, TestFETL,

testing::Combine(testing::Values(nTuple<3, Real>( { 40, -2.0, 0.0 })  //
//        , nTuple<3, Real>( { -1.0, -2.0, -3.0 })
        ),

testing::Values(

nTuple<3, Real>( { 42.0, 2.0, PI })  //
        , nTuple<3, Real>( { 51.0, 2.0, 0.0 }) //
        , nTuple<3, Real>( { 51.0, 0.0, TWOPI }) //
        , nTuple<3, Real>( { 51.0, 2.0, TWOPI }) //

        ),

testing::Values(

nTuple<3, size_t>( { 100, 1, 1 }) //
        , nTuple<3, size_t>( { 41, 101, 1 }) //
        , nTuple<3, size_t>( { 32, 1, 30 }) //
        , nTuple<3, size_t>( { 32, 36, 20 })   //

        ),

testing::Values(nTuple<3, Real>( { TWOPI, 4 * PI, 2.0 }))

));
