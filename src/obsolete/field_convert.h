/*
 * field_convert.h
 *
 *  Created on: 2013年12月17日
 *      Author: salmon
 */

#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include <type_traits>

namespace simpla
{
template<typename TS> class CoRectMesh;
template<typename, int, typename > class Field;
template<typename, int> class Geometry;

namespace fetl_impl
{
template<typename TL>
void _mapto(TL const & l, TL *r)
{
	*r = l;
}

template<typename TL, typename TR>
typename std::enable_if<is_complex<TL>::value && !is_complex<TR>::value, void>::type _mapto(TL const & l, TR *r)
{
	*r = l.real();
}

template<typename TL, typename TR, int N>
typename std::enable_if<is_complex<TL>::value && !is_complex<TR>::value, void>::type _mapto(nTuple<N, TL> const & l,
        nTuple<N, TR> *r)
{
	for (int i = 0; i < N; ++i)
		(*r)[i] = l[i].real();
}

template<typename TL, typename TR>
typename std::enable_if<is_complex<TR>::value, void>::type _mapto(TL const & l, TR *r)
{
	*r = l;
}

template<typename TL, typename TR, int N>
typename std::enable_if<is_complex<TR>::value, void>::type _mapto(nTuple<N, TL> const & l, nTuple<N, TR> *r)
{
	for (int i = 0; i < N; ++i)
		(*r)[i] = l[i];
}

}

template<typename TS, typename TL, typename TR>
void MapTo(Field<CoRectMesh<TS>, EDGE, TL> const & l, Field<CoRectMesh<TS>, VERTEX, nTuple<3, TR>> * r)
{
	r->Init();

	typedef CoRectMesh<TS> mesh_type;

	mesh_type const &mesh = r->mesh;

	auto const &dims = mesh.GetDimension();

	typedef typename mesh_type::index_type index_type;

	mesh.ParallelTraversal(VERTEX,

	[&](int m, index_type const &x,index_type const &y,index_type const &z)
	{
		auto &v =r->get(0,x,y,z);

		fetl_impl::_mapto((mesh.get(l,0,x,y,z)+mesh.get(l,0,x-1,y,z))*0.5,&(v[0]));
		fetl_impl::_mapto((mesh.get(l,1,x,y,z)+mesh.get(l,1,x,y-1,z))*0.5,&(v[1]));
		fetl_impl::_mapto((mesh.get(l,2,x,y,z)+mesh.get(l,2,x,y,z-1))*0.5,&(v[2]));
	});

}

template<typename TS, typename TL, typename TR>
void MapTo(Field<CoRectMesh<TS>, VERTEX, nTuple<3, TL>> const & l, Field<CoRectMesh<TS>, EDGE, TR> * r)
{
	r->Init();

	typedef CoRectMesh<TS> mesh_type;

	mesh_type const &mesh = l.mesh;

	auto const &dims = mesh.GetDimension();

	typedef typename mesh_type::index_type index_type;

	mesh.ParallelTraversal(VERTEX,

	[&](int m, index_type const &x,index_type const &y,index_type const &z)
	{

		fetl_impl::_mapto((mesh.get(l,0,x,y,z)[0]+mesh.get(l,0,x+1,y,z)[0])*0.5, &(r->get(0,x,y,z)));

		fetl_impl::_mapto((mesh.get(l,0,x,y,z)[1]+mesh.get(l,0,x,y+1,z)[1])*0.5, &(r->get(1,x,y,z)));

		fetl_impl::_mapto((mesh.get(l,0,x,y,z)[2]+mesh.get(l,0,x,y,z+1)[2])*0.5, &(r->get(2,x,y,z)));
	}

	);

}

template<typename TS, typename TL, typename TR>
void MapTo(Field<CoRectMesh<TS>, FACE, TL> const & l, Field<CoRectMesh<TS>, VERTEX, nTuple<3, TR>> * r)
{
	r->Init();

	typedef CoRectMesh<TS> mesh_type;

	mesh_type const &mesh = r->mesh;

	auto const &dims = mesh.GetDimension();

	typedef typename mesh_type::index_type index_type;

	mesh.ParallelTraversal(VERTEX,

	[&](int m, index_type const &x,index_type const &y,index_type const &z)
	{
		auto &v =r->get(0,x,y,z);
		for(int i=0;i<3;++i)
		{

			fetl_impl::_mapto((

							mesh.get(l,i,x,y,z)

							+ mesh.get(l,i,mesh.Shift(mesh.DES((i+1)),x,y,z))

							+ mesh.get(l,i,mesh.Shift(mesh.DES((i+2)),x,y,z))

							+ mesh.get(l,i,mesh.Shift(mesh.DES((i+1))|mesh.DES((i+2)),x,y,z))

					)*0.25,&(v[i]));

		}
	});

}

template<typename TS, typename TL, typename TR>
void MapTo(Field<CoRectMesh<TS>, VERTEX, nTuple<3, TL>> const & l, Field<CoRectMesh<TS>, FACE, TR> * r)
{
	r->Init();

	typedef CoRectMesh<TS> mesh_type;

	mesh_type const &mesh = l.mesh;

	auto const &dims = mesh.GetDimension();

	typedef typename mesh_type::index_type index_type;

	mesh.ParallelTraversal(VERTEX,

	[&](int m, index_type const &x,index_type const &y,index_type const &z)
	{
		for(int i=0;i<3;++i)
		{

			fetl_impl::_mapto((

							mesh.get(l,0,x,y,z)[i]

							+mesh.get(l,0,mesh.Shift(mesh.INC((i+1)),x,y,z))[i]

							+mesh.get(l,0,mesh.Shift(mesh.INC((i+2)),x,y,z))[i]

							+mesh.get(l,0,mesh.Shift(mesh.INC((i+1))|mesh.INC((i+2)),x,y,z))[i]

					)*0.25,&(r->get(i,x,y,z)));

		}
	}

	);

}

template<typename TS, typename TL, typename TR>
void MapTo(Field<CoRectMesh<TS>, EDGE, TL> const & l, Field<CoRectMesh<TS>, FACE, TR> * r)
{
	r->Init();

	typedef CoRectMesh<TS> mesh_type;

	mesh_type const &mesh = l.mesh;

	auto const &dims = mesh.GetDimension();

	typedef typename mesh_type::index_type index_type;

	mesh.ParallelTraversal(FACE,

	[&](int m, index_type const &x,index_type const &y,index_type const &z)
	{

		fetl_impl::_mapto((

						mesh.get(l,m,x,y,z)

						+mesh.get(l,m,mesh.Shift(mesh.INC((m+1)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.INC((m+2)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.INC((m+1))|mesh.INC((m+2)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.DES((m)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.DES((m))|mesh.INC((m+1)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.DES((m))|mesh.INC((m+2)),x,y,z))

						+mesh.get(l,m,mesh.Shift(mesh.DES((m))|mesh.INC((m+1))|mesh.INC((m+2)),x,y,z))

				)*0.125,&(r->get(m,x,y,z)));

	}

	);

}

//template<int IFORM, typename TS, typename TL, typename TR>
//void MapToReal(Field<Geometry<CoRectMesh<TS>, IFORM>, TL> const & l, Field<Geometry<CoRectMesh<TS>, IFORM>, TR> * r)
//{
//	r->Init();
//
//	*r = l;
//
//}
//
//template<int IFORM, typename TS, typename TL>
//void MapToReal(Field<Geometry<CoRectMesh<TS>, IFORM>, std::complex<TL> > const & l,
//        Field<Geometry<CoRectMesh<TS>, IFORM>, TL> * r)
//{
//	r->Init();
//
//	typedef CoRectMesh<TS> mesh_type;
//
//	mesh_type const &mesh = l.mesh;
//
//	auto const &dims = mesh.GetDimension();
//
//	typedef typename mesh_type::index_type index_type;
//
//	mesh.ParallelTraversal(IFORM, [&]( index_type const &s)
//	{	(*r)[s]=l[s].real();});
//
//}
//
//template<int IFORM, int N, typename TS, typename TL, typename TR>
//void MapToReal(Field<Geometry<CoRectMesh<TS>, IFORM>, nTuple<N, std::complex<TL>>> const& l,
//Field<Geometry<CoRectMesh<TS>, IFORM>, nTuple<N, TR>> * r)
//{
//	r->Init();
//
//	typedef CoRectMesh<TS> mesh_type;
//
//	mesh_type const &mesh = l.mesh;
//
//	auto const &dims = mesh.GetDimension();
//
//	typedef typename mesh_type::index_type index_type;
//
//	mesh.ParallelTraversal(IFORM, [&]( index_type const &s)
//	{
//		for(int i=0;i<N;++i)
//		(*r)[s][i]=l[s][i].real();
//	});
//
//}
////****************************************************************************************************
//namespace fetl_impl
//{
//
//template<typename T> inline constexpr T _real(T const &v)
//{
//	return v;
//}
//template<typename T> inline constexpr T _imag(T const &)
//{
//	return 0;
//}
//
//template<typename T> inline constexpr T _real(std::complex<T> const &v)
//{
//	return v.real();
//}
//template<typename T> inline constexpr T _imag(std::complex<T> const &v)
//{
//	return v.imag();
//}
//
//template<typename T> inline constexpr nTuple<3, T> _real(nTuple<3, std::complex<T>> const &v)
//{
//	return std::move(nTuple<3, T>( { v[0].real(), v[1].real(), v[2].real() }));
//}
//
//template<typename T> inline constexpr nTuple<3, T> _imag(nTuple<3, std::complex<T>> const &v)
//{
//	return std::move(nTuple<3, T>( { v[0].imag(), v[1].imag(), v[2].imag() }));
//}
//
//template<typename T> inline constexpr nTuple<3, s> _imag(nTuple<3, T> const &v)
//{
//	return std::move(nTuple<3, T>( { 0, 0, 0 }));
//}
//
//template<typename TGeo, typename TL, typename ...TI> inline auto FieldOpEval(Int2Type<REAL>, //
//        Field<TGeo, TL> const &l, TI ... s)
//        DECL_RET_TYPE((_real(l.get(s...))) )
//template<typename TGeo, typename TL, typename ...TI> inline auto FieldOpEval(Int2Type<IMAGINE>,
//        Field<TGeo, TL> const &l, TI ... s)
//        DECL_RET_TYPE((_imag(l.get(s...))) )
//
//}
//
//// STL style
//template<typename TM, int IFORM, typename TL>
//inline auto real(Field<Geometry<TM, IFORM>, TL> const & f)
//DECL_RET_TYPE( (Field<Geometry<TM, IFORM>, UniOp<REAL,Field<Geometry<TM, IFORM>, TL> > >(f)) )
//
//template<typename TM, int IFORM, typename TL>
//inline auto imag(Field<Geometry<TM, IFORM>, TL> const & f)
//DECL_RET_TYPE( (Field<Geometry<TM, IFORM>, UniOp<IMAGINE,Field<Geometry<TM, IFORM>, TL> > >(f)) )

}
// namespace simpla

#endif /* FIELD_CONVERT_H_ */