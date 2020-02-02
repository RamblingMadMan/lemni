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

#define LEMNI_NO_CPP
#include "AInt.hpp"
#include "ARatio.hpp"

namespace {
	LemniARatio createARatio(){
		auto mem = std::malloc(sizeof(LemniARatioT));
		return new(mem) LemniARatioT;
	}
}

LemniARatio lemniCreateARatio(void){
	auto p = createARatio();
	mpq_init(p->val);
	return p;
}

LemniARatio lemniCreateARatioStr(LemniStr str, const int base){
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

void lemniDestroyARatio(LemniARatio aratio){
	mpq_clear(aratio->val);
	std::destroy_at(aratio);
	std::free(aratio);
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
