/*
 *  _fetl_impl::vector_calculus.h
 *
 *  Created on: 2012-3-1
 *      Author: salmon
 */

#ifndef VECTOR_CALCULUS_H_
#define VECTOR_CALCULUS_H_
#include "fetl_defs.h"

namespace simpla
{

namespace _impl
{
template<typename TL, typename TR> struct OpDot
{
	typedef typename _impl::TypeConvertTraits<
			typename _impl::nTupleValueTraits<
					typename _impl::ValueTraits<TL>::Value>::Value,
			typename _impl::nTupleValueTraits<
					typename _impl::ValueTraits<TR>::Value>::Value>::Value Value;
};

template<typename TL, typename TR> struct OpCross
{
	typedef nTuple<THREE,
			typename _impl::TypeConvertTraits<
					typename _impl::nTupleValueTraits<
							typename _impl::ValueTraits<TL>::Value>::Value,
					typename _impl::nTupleValueTraits<
							typename _impl::ValueTraits<TR>::Value>::Value>::Value> Value;
	;
};


} // namespace namespace _impl{

template<typename TG, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpDot<Field<TG, IZeroForm, TLExpr>, Field<TG, IZeroForm, TRExpr> > >  //
Dot(Field<TG, IZeroForm, TLExpr> const & lhs,
		Field<TG, IZeroForm, TRExpr> const & rhs)
{
	return (Field<TG, IZeroForm,
			_impl::OpDot<Field<TG, IZeroForm, TLExpr>,
					Field<TG, IZeroForm, TRExpr> > >(lhs, rhs));
}

template<typename TG, int N, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpDot<nTuple<N, TLExpr>, Field<TG, IZeroForm, TRExpr> > >      //
Dot(nTuple<N, TLExpr> const & lhs, Field<TG, IZeroForm, TRExpr> const &rhs)
{

	return (Field<TG, IZeroForm,
			_impl::OpDot<nTuple<N, TLExpr>, Field<TG, IZeroForm, TRExpr> > >(
			lhs, rhs));
}

template<typename TG, int N, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpDot<Field<TG, IZeroForm, TLExpr>, nTuple<N, TRExpr> > >      //
Dot(Field<TG, IZeroForm, TLExpr> const & lhs, nTuple<N, TRExpr> const & rhs)
{

	return (Field<TG, IZeroForm,
			_impl::OpDot<Field<TG, IZeroForm, TLExpr>, nTuple<N, TRExpr> > >(
			lhs, rhs));
}

template<typename TG, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpCross<Field<TG, IZeroForm, TLExpr>,
				Field<TG, IZeroForm, TRExpr> > >                              //
Cross(Field<TG, IZeroForm, TLExpr> const & lhs,
		Field<TG, IZeroForm, TRExpr> const & rhs)
{
	return (Field<TG, IZeroForm,
			_impl::OpCross<Field<TG, IZeroForm, TLExpr>,
					Field<TG, IZeroForm, TRExpr> > >(lhs, rhs));
}

template<typename TG, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpCross<nTuple<THREE, TLExpr>, Field<TG, IZeroForm, TRExpr> > >  //
Cross(nTuple<THREE, TLExpr> const & lhs,
		Field<TG, IZeroForm, TRExpr> const &rhs)
{
	return (Field<TG, IZeroForm,
			_impl::OpCross<nTuple<THREE, TLExpr>, Field<TG, IZeroForm, TRExpr> > >(
			lhs, rhs));
}

template<typename TG, typename TLExpr, typename TRExpr>
inline Field<TG, IZeroForm,
		_impl::OpCross<Field<TG, IZeroForm, TLExpr>, nTuple<THREE, TRExpr> > >  //
Cross(Field<TG, IZeroForm, TLExpr> const & lhs,
		nTuple<THREE, TRExpr> const & rhs)
{
	return (Field<TG, IZeroForm,
			_impl::OpCross<Field<TG, IZeroForm, TLExpr>, nTuple<THREE, TRExpr> > >(
			lhs, rhs));
}

template<typename TG, typename TLExpr>
inline Field<TG, IOneForm,
		_impl::OpGrad<Field<TG, IZeroForm, TLExpr>, typename TG::DX_Type> >   //
Grad(Field<TG, IZeroForm, TLExpr> const & lhs)
{
	return (Field<TG, IOneForm,
			_impl::OpGrad<Field<TG, IZeroForm, TLExpr>, typename TG::DX_Type> >(
			lhs, typename TG::DX_Type()));
}

template<typename TG, typename TLExpr>
inline Field<TG, IZeroForm,
		_impl::OpDiverge<Field<TG, IOneForm, TLExpr>, typename TG::DX_Type> >  //
Diverge(Field<TG, IOneForm, TLExpr> const & lhs)
{
	return (Field<TG, IZeroForm,
			_impl::OpDiverge<Field<TG, IOneForm, TLExpr>, typename TG::DX_Type> >(
			lhs, typename TG::DX_Type()));
}

template<typename TG, typename TLExpr>
inline Field<TG, ITwoForm,
		_impl::OpCurl<Field<TG, IOneForm, TLExpr>, typename TG::DX_Type> >    //
Curl(Field<TG, IOneForm, TLExpr> const & lhs)
{
	return (Field<TG, ITwoForm,
			_impl::OpCurl<Field<TG, IOneForm, TLExpr>, typename TG::DX_Type> >(
			lhs, typename TG::DX_Type()));
}

template<typename TG, typename TLExpr>
inline Field<TG, IOneForm,
		_impl::OpCurl<Field<TG, ITwoForm, TLExpr>, typename TG::DX_Type> >    //
Curl(Field<TG, ITwoForm, TLExpr> const & lhs)
{
	return (Field<TG, IOneForm,
			_impl::OpCurl<Field<TG, ITwoForm, TLExpr>, typename TG::DX_Type> >(
			lhs,typename TG::DX_Type()));
}

template<int IPD, typename TG, typename TLExpr>
inline Field<TG, ITwoForm,
		_impl::OpCurlPD<Int2Type<IPD>, Field<TG, IOneForm, TLExpr> > >        //
CurlPD(Int2Type<IPD>, Field<TG, IOneForm, TLExpr> const & rhs)
{
	return (Field<TG, ITwoForm,
			_impl::OpCurlPD<Int2Type<IPD>, Field<TG, IOneForm, TLExpr> > >(
			Int2Type<IPD>(), rhs));
}

template<int IPD, typename TG, typename TLExpr>
inline Field<TG, IOneForm,
		_impl::OpCurlPD<Int2Type<IPD>, Field<TG, ITwoForm, TLExpr> > >        //
CurlPD(Int2Type<IPD>, Field<TG, ITwoForm, TLExpr> const & rhs)
{
	return (Field<TG, IOneForm,
			_impl::OpCurlPD<Int2Type<IPD>, Field<TG, ITwoForm, TLExpr> > >(
			Int2Type<IPD>(), rhs));
}

template<typename TG, int IL, int IR, typename TRExpr>
inline Field<TG, IL, _impl::OpMapTo<Int2Type<IL>, Field<TG, IR, TRExpr> > >   //
MapTo(Int2Type<IL>, Field<TG, IR, TRExpr> const & rhs)
{
	return (Field<TG, IL, _impl::OpMapTo<Int2Type<IL>, Field<TG, IR, TRExpr> > >(
			Int2Type<IL>(), rhs));
}

}        // namespace simpla
#endif /* VECTOR_CALCULUS_H_ */
