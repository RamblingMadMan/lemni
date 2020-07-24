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
#include <cstring>

#include <new>
#include <memory>
#include <string>

#define LEMNI_NO_CPP
#include "AInt.hpp"

LemniAInt lemniCreateAInt(void){
	auto p = createAInt();
	mpz_init(p->val);
	return p;
}

LemniAInt lemniCreateAIntCopy(LemniAIntConst other){
	auto p = createAInt();
	mpz_init_set(p->val, other->val);
	return p;
}

LemniAInt lemniCreateAIntStr(const LemniStr str, const int base){
	auto p = createAInt();
	std::string cstr(str.ptr, str.len);
	mpz_init_set_str(p->val, cstr.c_str(), base);
	return p;
}

LemniAInt lemniCreateAIntLong(const long si){
	auto p = createAInt();
	mpz_init_set_si(p->val, si);
	return p;
}

LemniAInt lemniCreateAIntULong(const unsigned long ui){
	auto p = createAInt();
	mpz_init_set_ui(p->val, ui);
	return p;
}

void lemniDestroyAInt(LemniAInt aint){
	mpz_clear(aint->val);
	std::destroy_at(aint);
	std::free(aint);
}

void lemniAIntSet(LemniAInt aint, LemniAIntConst other){
	mpz_set(aint->val, other->val);
}

void lemniAIntSetStr(LemniAInt aint, LemniStr str, const int base){
	std::string cstr(str.ptr, str.len);
	mpz_set_str(aint->val, cstr.c_str(), base);
}

void lemniAIntSetLong(LemniAInt aint, const long si){
	mpz_set_si(aint->val, si);
}

void lemniAIntSetULong(LemniAInt aint, const unsigned long ui){
	mpz_set_ui(aint->val, ui);
}

uint64_t lemniAIntNumBits(LemniAIntConst aint){
	return mpz_sizeinbase(aint->val, 2) + 1;
}

uint64_t lemniAIntNumBitsUnsigned(LemniAIntConst aint){
	return mpz_sizeinbase(aint->val, 2);
}

long lemniAIntToLong(LemniAIntConst aint){
	return mpz_get_si(aint->val);
}

unsigned long lemniAIntToULong(LemniAIntConst aint){
	return mpz_get_ui(aint->val);
}

void lemniAIntStr(LemniAIntConst aint, void *user, LemniAIntStrCB cb){
	auto size = mpz_sizeinbase(aint->val, 10) + 2;
	auto chars = std::make_unique<char[]>(size);
	std::memset(chars.get(), 0, size);
	mpz_get_str(chars.get(), 10, aint->val);
	cb(user, {chars.get(), strnlen(chars.get(), size-1)});
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

void lemniAIntPow(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs){
}

void lemniAIntNeg(LemniAInt res, LemniAIntConst val){
	mpz_neg(res->val, val->val);
}

void lemniAIntAbs(LemniAInt res, LemniAIntConst val){
	mpz_abs(res->val, val->val);
}

int lemniAIntCmp(LemniAIntConst lhs, LemniAIntConst rhs){
	return mpz_cmp(lhs->val, rhs->val);
}
