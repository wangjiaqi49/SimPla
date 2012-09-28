/*Copyright (C) 2007-2011 YU Zhi. All rights reserved.
 *
 * $Id: FETL.h 1009 2011-02-07 23:20:45Z salmon $
 * FETL.h
 *
 *  Created on: 2009-3-31
 *      Author: salmon
 */
#ifndef FETL_H_
#define FETL_H_

#include "fetl/fetl_defs.h"
#ifndef DEFAULT_GRID
#  include "grid/uniform_rect.h"
#endif

namespace simpla
{
namespace fetl {

//Default fields are real value

template<int IFORM, typename TV, typename TG = DEFAULT_GRID> struct Field;

#define DEFINE_FIELDS(TV,TG)                                          \
typedef Field<IZeroForm, TV,TG> ZeroForm;                             \
typedef Field<IOneForm, TV,TG> OneForm;                               \
typedef Field<ITwoForm, TV,TG> TwoForm;                               \
typedef Field<IThreeForm, TV,TG> ThreeForm;                           \
                                                                      \
typedef Field<IZeroForm, Vec3,TG> VecZeroForm;                        \
typedef Field<IOneForm, Vec3,TG> VecOneForm;                          \
typedef Field<ITwoForm, Vec3,TG> VecTwoForm;                          \
typedef Field<IThreeForm, Vec3,TG> VecThreeForm;                      \
                                                                      \
typedef Field<IZeroForm, TV,TG> ScalarField;                          \
typedef Field<IZeroForm, Vec3,TG> VecField;                           \
                                                                      \
typedef Field<IZeroForm, TV,TG> RZeroForm;                            \
typedef Field<IOneForm, TV,TG> ROneForm;                              \
typedef Field<ITwoForm, TV,TG> RTwoForm;                              \
typedef Field<IThreeForm, TV,TG> RThreeForm;                          \
                                                                      \
typedef Field<IZeroForm, RVec3,TG> RVecZeroForm;                      \
typedef Field<IOneForm, RVec3,TG> RVecOneForm;                        \
typedef Field<ITwoForm, RVec3,TG> RVecTwoForm;                        \
typedef Field<IThreeForm, RVec3,TG> RVecThreeForm;                    \
                                                                      \
typedef Field<IZeroForm, TV,TG> RScalarField;                         \
typedef Field<IZeroForm, RVec3,TG> RVecField;                         \
                                                                      \
typedef Field<IZeroForm, Complex,TG> CZeroForm;                       \
typedef Field<IOneForm, Complex,TG> COneForm;                         \
typedef Field<ITwoForm, Complex,TG> CTwoForm;                         \
typedef Field<IZeroForm, CVec3,TG> CVecZeroForm;                      \
typedef Field<IZeroForm, Complex,TG> CScalarField;                    \
typedef Field<IZeroForm, CVec3,TG> CVecField;                         \


}  // namespace fetl
} //namespace simpla
#include "fetl/fields.h"
#include "fetl/arithmetic.h"
#include "fetl/vector_calculus.h"

#endif  // FETL_H_
