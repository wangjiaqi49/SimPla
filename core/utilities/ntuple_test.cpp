/*
 * testnTuple.cpp
 *
 *  created on: 2012-1-10
 *      Author: salmon
 */

#include <gtest/gtest.h>

#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
//#include "expression_template.h"
#include "sp_type_traits.h"
#include "sp_integer_sequence.h"
#include "ntuple.h"
//#include "log.h"
#include "pretty_stream.h"
using namespace simpla;

int main(int argc, char **argv)
{
	nTuple<double, 3> a =
	{ 1, 2, 3 };

	std::cout << a << std::endl;
	a *= 2;
	std::cout << a << std::endl;

}

//
//
//#define EQUATION(_A,_B,_C)  ( -(_A  +TestFixture::a )/(   _B *TestFixture::b -TestFixture::c  )- _C)
//
//template<typename T>
//class TestNtuple: public testing::Test
//{
//protected:
//
//	virtual void SetUp()
//	{
//
//		a = 1;
//		b = 3;
//		c = 4;
//		d = 7;
//
//		DIMENSIONS = seq2ntuple(dimensions());
//
//		seq_for_each(dimensions(),
//
//		[&](size_t const idx[dimensions::size()])
//		{
//			get_value_r(aA,idx) = idx[0] * 2;
//			get_value_r(aB,idx) = 5 - idx[0];
//			get_value_r(aC,idx) = idx[0] * 5 + 1;
//			get_value_r(aD,idx) = 0;
//			get_value_r(vA,idx) = get_value_r(aA,idx);
//			get_value_r(vB,idx) = get_value_r(aB,idx);
//			get_value_r(vC,idx) = get_value_r(aC,idx);
//			get_value_r(vD,idx) = 0;
//
//			get_value_r(res,idx) = -(get_value_r(aA,idx) + a) /
//			(get_value_r(aB,idx) * b - c) - get_value_r(aC,idx);
//
//		});
//
//		num_of_loops = 1000000L;
//	}
//
//public:
//
//	std::size_t num_of_loops = 10000000L;
//
//	typedef T type;
//
//	typedef typename nTuple_traits<type>::dimensions dimensions;
//
//	nTuple<std::size_t, dimensions::size()> DIMENSIONS;
//
//	typedef typename type::value_type value_type;
//
//	type vA, vB, vC, vD;
//
//	typename nTuple_traits<T>::pod_type aA, aB, aC, aD, res;
//
//	value_type a, b, c, d;
//
//};
//
//typedef testing::Types<
//
//nTuple<double, 3>
//
//, nTuple<double, 3, 3>
//
//, nTuple<double, 3, 4, 5>
//
//, nTuple<int, 3, 4, 5, 6>
//
//, nTuple<std::complex<double>, 3>
//
//> nTupleTypes;
//
//TYPED_TEST_CASE(TestNtuple, nTupleTypes);
//
//TYPED_TEST(TestNtuple, swap){
//{
//
//	swap(TestFixture::vA, TestFixture::vB);
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				EXPECT_DOUBLE_EQ(0, abs(get_value_r(TestFixture::aA,idx)- get_value_r(TestFixture:: vB,idx)));
//				EXPECT_DOUBLE_EQ(0, abs(get_value_r(TestFixture::aB,idx)- get_value_r(TestFixture:: vA,idx)));
//			});
//
//}
//}
//
//TYPED_TEST(TestNtuple, reduce){
//{
//	typename TestFixture::value_type expect=0;
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				expect+=get_value_r(TestFixture::vA,idx);
//			}
//	);
//	auto value=seq_reduce(typename TestFixture::dimensions(), _impl::plus(), TestFixture::vA);
//
//	EXPECT_DOUBLE_EQ(0,abs(expect -value));
//
//}
//}
//
//TYPED_TEST(TestNtuple, compare){
//{
////	EXPECT_TRUE( TestFixture::vA==TestFixture::aA);
//	EXPECT_FALSE( TestFixture::vA==TestFixture::vB);
//
//	EXPECT_TRUE( TestFixture::vA!=TestFixture::vB);
//	EXPECT_FALSE( TestFixture::vA!=TestFixture::vA);
//
//}
//}
////
//TYPED_TEST(TestNtuple, Assign_Scalar){
//{
//
//	TestFixture::vA = TestFixture::a;
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				EXPECT_DOUBLE_EQ(0, abs(TestFixture::a- get_value_r(TestFixture:: vA,idx)));
//			}
//	);
//
//}}
//
//TYPED_TEST(TestNtuple, Assign_Array){
//{
//	TestFixture::vA = TestFixture::aA;
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				EXPECT_DOUBLE_EQ(0, abs(get_value_r(TestFixture::aA,idx)- get_value_r(TestFixture:: vA,idx)));
//			}
//	);
//
//}}
//
//TYPED_TEST(TestNtuple, Arithmetic){
//{
//	TestFixture::vD = EQUATION(TestFixture::vA, TestFixture::vB, TestFixture::vC);
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				auto &ta=get_value_r(TestFixture::vA,idx);
//				auto &tb=get_value_r(TestFixture::vB,idx);
//				auto &tc=get_value_r(TestFixture::vC,idx);
//				auto &td=get_value_r(TestFixture::vD,idx);
//
//				EXPECT_DOUBLE_EQ(0, abs(EQUATION(ta,tb,tc ) - td));
//			}
//	);
//
//}
//}
//
//TYPED_TEST(TestNtuple, self_assign){
//{
//	TestFixture::vB +=TestFixture::vA;
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				EXPECT_DOUBLE_EQ(0,abs( get_value_r(TestFixture::vB,idx)
//								- (get_value_r(TestFixture::aB,idx)+
//										get_value_r(TestFixture::aA,idx))));
//
//			}
//	);
//
//}
//}
//
//TYPED_TEST(TestNtuple, inner_product){
//{
//	typename TestFixture::value_type res;
//
//	res=0;
//
//	seq_for_each(typename TestFixture::dimensions(),
//			[&](size_t const idx[TestFixture::dimensions::size()])
//			{
//				res += get_value_r(TestFixture::aA,idx) * get_value_r(TestFixture::aB,idx);
//			}
//	);
//
//	EXPECT_DOUBLE_EQ(0,abs(res- inner_product( TestFixture::vA, TestFixture::vB)));
//}}
//
//TYPED_TEST(TestNtuple, Cross){
//{
//	nTuple< typename TestFixture::value_type,3> vA, vB,vC ,vD;
//
//	for (int i = 0; i < 3; ++i)
//	{
//		vA[i] = (i * 2);
//		vB[i] = (5 - i);
//	}
//
//	for (int i = 0; i < 3; ++i)
//	{
//		vD[i] = vA[(i + 1) % 3] * vB[(i + 2) % 3]
//		- vA[(i + 2) % 3] * vB[(i + 1) % 3];
//	}
//
//	vC=cross(vA,vB);
//	vD-=vC;
//	EXPECT_DOUBLE_EQ(0,abs(vD[0])+abs(vD[1])+abs(vD[2]) );
//}}
//
////TYPED_TEST(TestNtuple, performance_rawarray){
////{
////	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
////	{
////		seq_for_each(typename TestFixture::dimensions(),
////				[&](size_t const idx[TestFixture::dimensions::size()])
////				{
////					get_value_r(TestFixture::aD,idx) +=EQUATION(get_value_r(TestFixture::aA,idx),
////							get_value_r(TestFixture::aB,idx),
////							get_value_r(TestFixture::aC,idx))
////					*s;
////
////				}
////		)
////
////		;
////	}
////
////}
////}
////TYPED_TEST(TestNtuple, performancenTuple){
////{
////
////	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
////	{
////		TestFixture::vD +=EQUATION(TestFixture::vA ,TestFixture::vB ,TestFixture::vC)*(s);
////	}
////
////}
////}
//
//template<typename > class nTuplePerf1;
//
//template<typename T, size_t N>
//class nTuplePerf1<nTuple<T, N>> : public testing::Test
//{
//	virtual void SetUp()
//	{
//
//		a = 1;
//		b = 3;
//		c = 4;
//		d = 7;
//
//		dims0 = N;
//
//		for (int i = 0; i < dims0; ++i)
//		{
//			aA[i] = i * 2;
//			aB[i] = 5 - i;
//			aC[i] = i * 5 + 1;
//			aD[i] = 0;
//			vA[i] = aA[i];
//			vB[i] = aB[i];
//			vC[i] = aC[i];
//			vD[i] = 0;
//
//			res[i] = -(aA[i] + a) / (aB[i] * b - c) - aC[i];
//		}
//
//	}
//public:
//	std::size_t num_of_loops = 10000000L;
//
//	typedef nTuple<T, N> type;
//
//	typedef T value_type;
//
//	type vA, vB, vC, vD;
//
//	value_type aA[N], aB[N], aC[N], aD[N], res[N];
//
//	value_type a, b, c, d;
//
//	size_t dims0, dims1;
//
//};
//typedef testing::Types<
//
//nTuple<double, 3>
//
//, nTuple<double, 20>
//
//, nTuple<int, 3>
//
//, nTuple<int, 10>
//
//, nTuple<std::complex<double>, 3>
//
//, nTuple<std::complex<double>, 10>
//
//> nTuple_types1;
//
//TYPED_TEST_CASE(nTuplePerf1, nTuple_types1);
//
//TYPED_TEST(nTuplePerf1,performance_raw_array){
//{
//	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
//	{
//		for (int i = 0; i < TestFixture::dims0; ++i)
//		{
//			TestFixture::aD[i] += EQUATION( TestFixture::aA[i], TestFixture::aB[i], TestFixture::aC[i]) * s;
//		}
//
//	}
//}
//}
//
//TYPED_TEST(nTuplePerf1, performancenTuple){
//{
//
//	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
//	{
//		TestFixture::vD +=EQUATION(TestFixture::vA ,TestFixture::vB ,TestFixture::vC)*(s);
//	}
//
//}
//}
//
//template<typename > class nTuplePerf2;
//
//template<typename T, size_t N, size_t M>
//class nTuplePerf2<nTuple<T, N, M>> : public testing::Test
//{
//	virtual void SetUp()
//	{
//
//		a = 1;
//		b = 3;
//		c = 4;
//		d = 7;
//
//		dims0 = N;
//		dims1 = M;
//
//		for (int i = 0; i < dims0; ++i)
//			for (int j = 0; j < dims1; ++j)
//			{
//				aA[i][j] = i * 2;
//				aB[i][j] = 5 - i;
//				aC[i][j] = i * 5 + 1;
//				aD[i][j] = 0;
//				vA[i][j] = aA[i][j];
//				vB[i][j] = aB[i][j];
//				vC[i][j] = aC[i][j];
//				vD[i][j] = 0;
//
//				res[i][j] = -(aA[i][j] + a) / (aB[i][j] * b - c) - aC[i][j];
//			}
//
//	}
//public:
//	std::size_t num_of_loops = 10000000L;
//
//	typedef nTuple<T, M, N> type;
//
//	typedef T value_type;
//
//	type vA, vB, vC, vD;
//
//	value_type aA[N][M], aB[N][M], aC[N][M], aD[N][M], res[N][M];
//
//	value_type a, b, c, d;
//
//	size_t dims0, dims1;
//
//};
//typedef testing::Types<
//
//nTuple<double, 3, 4>
//
//, nTuple<double, 20, 10>
//
//, nTuple<int, 3, 4>
//
//, nTuple<int, 10, 20>
//
//, nTuple<std::complex<double>, 3, 4>
//
//, nTuple<std::complex<double>, 10, 20>
//
//> nTuple_types2;
//
//TYPED_TEST_CASE(nTuplePerf2, nTuple_types2);
//
//TYPED_TEST(nTuplePerf2,performance_raw_array){
//{
//	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
//	{
//		for (int i = 0; i < TestFixture::dims0; ++i)
//		for (int j = 0; j < TestFixture::dims1; ++j)
//		{
//			TestFixture::aD[i][j] += EQUATION( TestFixture::aA[i][j], TestFixture::aB[i][j], TestFixture::aC[i][j]) * s;
//		}
//
//	}
//}
//}
//
//TYPED_TEST(nTuplePerf2, performancenTuple){
//{
//
//	for (std::size_t s = 0; s < TestFixture::num_of_loops; ++s)
//	{
//		TestFixture::vD +=EQUATION(TestFixture::vA ,TestFixture::vB ,TestFixture::vC)*(s);
//	}
//
//}
//}
//
