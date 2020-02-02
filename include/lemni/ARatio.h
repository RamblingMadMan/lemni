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

#ifndef LEMNI_ARATIO_H
#define LEMNI_ARATIO_H 1

#include "AInt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniARatioT *LemniARatio;
typedef const struct LemniARatioT *LemniARatioConst;

LemniARatio lemniCreateARatio(void);
LemniARatio lemniCreateARatioStr(LemniStr str, const int base);
LemniARatio lemniCreateARatioAInt(LemniAIntConst num, LemniAIntConst den);
LemniARatio lemniCreateARatioLong(const long num, const unsigned long den);
LemniARatio lemniCreateARatioULong(const unsigned long num, const unsigned long den);

void lemniDestroyARatio(LemniARatio aratio);

void lemniARatioAdd(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);
void lemniARatioSub(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);
void lemniARatioMul(LemniARatio res, LemniARatioConst lhs, LemniARatio rhs);
void lemniARatioDiv(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);

void lemniARatioInv(LemniARatio res, LemniARatioConst val);
void lemniARatioNeg(LemniARatio res, LemniARatioConst val);
void lemniARatioAbs(LemniARatio res, LemniARatioConst val);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_ARATIO_H
