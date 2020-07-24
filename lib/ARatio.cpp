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
#include <string>

#define LEMNI_NO_CPP
#include "AInt.hpp"
#include "ARatio.hpp"

LemniARatio lemniCreateARatio(void){
	auto p = createARatio();
	mpq_init(p->val);
	return p;
}

LemniARatio lemniCreateARatioCopy(LemniARatioConst other){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set(p->val, other->val);
	return p;
}

LemniARatio lemniCreateARatioStr(const LemniStr str, const int base){
	auto p = createARatio();
	std::string cstr(str.ptr, str.len);
	mpq_init(p->val);
	mpq_set_str(p->val, cstr.c_str(), base);
	return p;
}

LemniARatio lemniCreateARatioAInt(LemniAIntConst num, LemniAIntConst den){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_num(p->val, num->val);
	mpq_set_den(p->val, den->val);
	return p;
}

LemniARatio lemniCreateARatioLong(const long num, const unsigned long den){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_si(p->val, num, den);
	return p;
}

LemniARatio lemniCreateARatioULong(const unsigned long num, const unsigned long den){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_ui(p->val, num, den);
	return p;
}

LemniARatio lemniCreateARatioFrom32(const LemniRatio32 q32){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_si(p->val, q32.num, q32.den);
	return p;
}

LemniARatio lemniCreateARatioFrom64(const LemniRatio64 q64){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_si(p->val, q64.num, q64.den);
	return p;
}

LemniARatio lemniCreateARatioFrom128(const LemniRatio128 q128){
	auto p = createARatio();
	mpq_init(p->val);
	mpq_set_si(p->val, q128.num, q128.den);
	return p;
}

LemniARatio lemniCreateARatioFrom64(const LemniRatio64 q64);

LemniARatio lemniCreateARatioFrom128(const LemniRatio128 q128);

void lemniDestroyARatio(LemniARatio aratio){
	mpq_clear(aratio->val);
	std::destroy_at(aratio);
	std::free(aratio);
}

void lemniARatioSet(LemniARatio res, LemniARatioConst other){
	mpq_set(res->val, other->val);
}

LemniRatio128 lemniARatioToRatio128(LemniARatioConst aratio){
	return {
		mpz_get_si(mpq_numref(aratio->val)),
		mpz_get_ui(mpq_denref(aratio->val))
	};
}

LemniARatioNumBitsResult lemniARatioNumBits(LemniARatioConst aratio){
	size_t numBits = mpz_sizeinbase(mpq_numref(aratio->val), 2);
	size_t denBits = mpz_sizeinbase(mpq_denref(aratio->val), 2);
	return { numBits + 1, denBits };
}

void lemniARatioStr(LemniARatioConst aratio, void *user, LemniARatioStrCB cb){
	mpq_canonicalize(const_cast<LemniARatio>(aratio)->val);
	auto lhsSize = mpz_sizeinbase(mpq_numref(aratio->val), 10) + 2;
	auto rhsSize = mpz_sizeinbase(mpq_denref(aratio->val), 10) + 1;
	auto chars = std::make_unique<char[]>(lhsSize + rhsSize + 1);
	mpz_get_str(chars.get(), lhsSize, mpq_numref(aratio->val));
	lhsSize = strnlen(chars.get(), lhsSize - 1);
	chars.get()[lhsSize++] = '/';
	mpz_get_str(chars.get() + lhsSize, rhsSize, mpq_denref(aratio->val));
	rhsSize = strnlen(chars.get() + lhsSize, rhsSize - 1);
	cb(user, {chars.get(), lhsSize + rhsSize});
}

LemniAInt lemniARatioNum(LemniARatioConst aratio){
	auto aint = createAInt();
	mpz_init_set(aint->val, mpq_numref(aratio->val));
	return aint;
}

LemniAInt lemniARatioDen(LemniARatioConst aratio){
	auto aint = createAInt();
	mpz_init_set(aint->val, mpq_denref(aratio->val));
	return aint;
}

void lemniARatioAdd(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs){
	mpq_add(res->val, lhs->val, rhs->val);
}

void lemniARatioSub(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs){
	mpq_sub(res->val, lhs->val, rhs->val);
}

void lemniARatioMul(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs){
	mpq_mul(res->val, lhs->val, rhs->val);
}

void lemniARatioDiv(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs){
	mpq_div(res->val, lhs->val, rhs->val);
}

void lemniARatioInv(LemniARatio res, LemniARatioConst val){
	mpq_inv(res->val, val->val);
}

void lemniARatioNeg(LemniARatio res, LemniARatioConst val){
	mpq_neg(res->val, val->val);
}

void lemniARatioAbs(LemniARatio res, LemniARatioConst val){
	mpq_abs(res->val, val->val);
}

int lemniARatioCmp(LemniARatioConst lhs, LemniARatioConst rhs){
	return mpq_cmp(lhs->val, rhs->val);
}
