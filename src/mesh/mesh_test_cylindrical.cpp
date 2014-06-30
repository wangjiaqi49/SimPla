/*
 * mesh_test.cpp
 *
 *  Created on: 2013年12月29日
 *      Author: salmon
 */

#include <gtest/gtest.h>

#include "../mesh/uniform_array.h"
#include "../mesh/geometry_cylindrical.h"
#include "../mesh/mesh_rectangle.h"

#define TMESH Mesh<CylindricalGeometry<UniformArray, false>>

#include "mesh_test.h"

using namespace simpla;

INSTANTIATE_TEST_CASE_P(SimPla, TestMesh,

testing::Combine(

testing::Values(nTuple<3, Real>(
{ 0.0, 0.0, 0.0, }) //
		, nTuple<3, Real>(
{ 1.0, 2.0, 0.0 })    //
		),

testing::Values(
//
		nTuple<3, Real>(
		{ 2.0, 0.0, 0.0 }), //
		nTuple<3, Real>(
		{ 0.0, 2.0, 0.0 }), //
		nTuple<3, Real>(
		{ 0.0, 0.0, 2.0 }), //
		nTuple<3, Real>(
		{ 0.0, 2.0, 2.0 }), //
		nTuple<3, Real>(
		{ 2.0, 0.0, 2.0 }), //
		nTuple<3, Real>(
		{ 2.0, 2.0, 0.0 }), //
		nTuple<3, Real>(
		{ 1.0, 3.0, 2.0 })  //

				),

testing::Values(

nTuple<3, size_t>(
{ 10, 1, 1 }) //
		, nTuple<3, size_t>(
{ 11, 1, 1 }) //
		, nTuple<3, size_t>(
{ 1, 17, 1 }) //
		, nTuple<3, size_t>(
{ 1, 1, 10 }) //
		, nTuple<3, size_t>(
{ 1, 17, 17 }) //
		, nTuple<3, size_t>(
{ 17, 1, 17 }) //
		, nTuple<3, size_t>(
{ 17, 17, 1 }) //
		, nTuple<3, size_t>(
{ 13, 16, 10 })   //

		)));