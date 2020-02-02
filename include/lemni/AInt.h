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

#ifndef LEMNI_AINT_H
#define LEMNI_AINT_H 1

#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniAIntT *LemniAInt;
typedef const struct LemniAIntT *LemniAIntConst;

LemniAInt lemniCreateAInt(void);
LemniAInt lemniCreateAIntStr(LemniStr str, const int base);
LemniAInt lemniCreateAIntLong(const long val);
LemniAInt lemniCreateAIntULong(const unsigned long val);

void lemniDestroyAInt(LemniAInt aint);

LemniStr lemniAIntStr(LemniAInt aint);

void lemniAIntAdd(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntSub(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntMul(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);

void lemniAIntNeg(LemniAInt res, LemniAIntConst val);
void lemniAIntAbs(LemniAInt res, LemniAIntConst val);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_AINT_H
