/*
	The Lemni Programming Language - Functional computer speak
	Copyright (C) 2020  Keith Hammond

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LEMNI_AREAL_H
#define LEMNI_AREAL_H 1

#include "ARatio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniARealT *LemniAReal;
typedef const struct LemniARealT *LemniARealConst;

LemniAReal lemniCreateAReal(void);
LemniAReal lemniCreateARealStr(LemniStr str, const int base);
LemniAReal lemniCreateARealAInt(LemniAIntConst aint);
LemniAReal lemniCreateARealARatio(LemniARatioConst aratio);
LemniAReal lemniCreateARealDouble(const double d);

void lemniDestroyAReal(LemniAReal areal);

void lemniARealAdd(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealSub(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealMul(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealDiv(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

void lemniARealNeg(LemniAReal res, LemniARealConst val);
void lemniARealAbs(LemniAReal res, LemniARealConst val);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_AREAL_H
