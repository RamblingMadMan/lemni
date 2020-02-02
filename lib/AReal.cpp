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

#include <cstdlib>
#include <new>
#include <memory>

#include "AInt.hpp"
#include "ARatio.hpp"
#include "AReal.hpp"

namespace {
	LemniAReal createAReal(){
		auto mem = std::malloc(sizeof(LemniARealT));
		return new(mem) LemniARealT;
	}
}

LemniAReal lemniCreateAReal(void){
	auto p = createAReal();
	mpfr_init(p->val);
	return p;
}

LemniAReal lemniCreateARealStr(LemniStr str, const int base){
	auto p = createAReal();
	std::string cstr(str.ptr, str.len);
	mpfr_init_set_str(p->val, cstr.c_str(), base, MPFR_RNDN);
	return p;
}

LemniAReal lemniCreateARealAInt(LemniAIntConst aint){
	auto p = createAReal();
	mpfr_init_set_z(p->val, aint->val, MPFR_RNDN);
	return p;
}

LemniAReal lemniCreateARealARatio(LemniARatioConst aratio){
	auto p = createAReal();
	mpfr_init_set_q(p->val, aratio->val, MPFR_RNDN);
	return p;
}

LemniAReal lemniCreateARealDouble(const double d){
	auto p = createAReal();
	mpfr_init_set_d(p->val, d, MPFR_RNDN);
	return p;
}

void lemniDestroyAReal(LemniAReal areal){
	mpfr_clear(areal->val);
	std::destroy_at(areal);
	std::free(areal);
}

void lemniARealAdd(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	mpfr_add(res->val, lhs->val, rhs->val, MPFR_RNDN);
}

void lemniARealSub(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	mpfr_sub(res->val, lhs->val, rhs->val, MPFR_RNDN);
}

void lemniARealMul(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	mpfr_mul(res->val, lhs->val, rhs->val, MPFR_RNDN);
}

void lemniARealDiv(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	mpfr_div(res->val, lhs->val, rhs->val, MPFR_RNDN);
}

void lemniARealNeg(LemniAReal res, LemniARealConst val){
	mpfr_neg(res->val, val->val, MPFR_RNDN);
}

void lemniARealAbs(LemniAReal res, LemniARealConst val){
	mpfr_abs(res->val, val->val, MPFR_RNDN);
}
