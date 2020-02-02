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

namespace {
	LemniAInt createAInt(){
		auto mem = std::malloc(sizeof(LemniAIntT));
		return new(mem) LemniAIntT;
	}
}

LemniAInt lemniCreateAInt(void){
	auto p = createAInt();
	mpz_init(p->val);
	return p;
}

LemniAInt lemniCreateAIntStr(LemniStr str, const int base){
	auto p = createAInt();
	std::string cstr(str.ptr, str.len);
	mpz_init_set_str(p->val, cstr.c_str(), base);
	return p;
}

LemniAInt lemniCreateAIntLong(const long val){
	auto p = createAInt();
	mpz_init_set_si(p->val, val);
	return p;
}

LemniAInt lemniCreateAIntULong(const unsigned long val){
	auto p = createAInt();
	mpz_init_set_ui(p->val, val);
	return p;
}

void lemniDestroyAInt(LemniAInt aint){
	mpz_clear(aint->val);
	std::destroy_at(aint);
	std::free(aint);
}

void lemniAIntAdd(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs){
	mpz_add(res->val, lhs->val, rhs->val);
}

void lemniAIntSub(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs){
	mpz_sub(res->val, lhs->val, rhs->val);
}

void lemniAIntMul(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs){
	mpz_mul(res->val, lhs->val, rhs->val);
}

void lemniAIntNeg(LemniAInt res, LemniAIntConst val){
	mpz_neg(res->val, val->val);
}

void lemniAIntAbs(LemniAInt res, LemniAIntConst val){
	mpz_abs(res->val, val->val);
}
