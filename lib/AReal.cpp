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
#include <cfloat>

#include <new>
#include <memory>
#include <numeric>

#include "lemni/Str.h"

#define LEMNI_NO_CPP
#include "AInt.hpp"
#include "ARatio.hpp"
#include "AReal.hpp"

template<typename T, void(*Init)(T), void(*Clear)(T)>
struct NumWrapper{
	NumWrapper(){ Init(val); }
	~NumWrapper(){ Clear(val); }

	operator T &() noexcept{ return val; }
	operator const T&() const noexcept{ return val; }

	T val;
};

using Mpfr = NumWrapper<mpfr_t, mpfr_init, mpfr_clear>;
using Arf = NumWrapper<arf_t, arf_init, arf_clear>;
using Arb = NumWrapper<arb_t, arb_init, arb_clear>;

LemniAReal lemniCreateAReal(void){
	return createAReal();
}

LemniAReal lemniCreateARealCopy(LemniARealConst other){
	auto p = createAReal();
	arb_set(p->val, other->val);
	return p;
}

LemniAReal lemniCreateARealStr(const LemniStr str, const LemniNat16 base){
	auto p = createAReal();

	std::string cstr(str.ptr, str.len);

	Mpfr mpfr;
	mpfr_set_str(mpfr, cstr.c_str(), base, MPFR_RNDN);

	Arf mid;
	arf_set_mpfr(mid, mpfr);

	arb_set_arf(p->val, mid);

	arb_trim(p->val, p->val);

	return p;
}

LemniAReal lemniCreateARealAInt(LemniAIntConst aint){
	auto p = createAReal();

	Arf valf;

	arf_set_mpz(valf, aint->val);

	arb_set_arf(p->val, valf);

	return p;
}

LemniAReal lemniCreateARealARatio(LemniARatioConst aratio){
	auto p = createAReal();

	Arf numf, denf;

	arf_set_mpz(numf, mpq_numref(aratio->val));
	arf_set_mpz(denf, mpq_denref(aratio->val));

	Arb num, den;

	arb_set_arf(num, numf);
	arb_set_arf(den, denf);

	arb_div(p->val, num, den, 53);

	return p;
}

LemniAReal lemniCreateARealDouble(const double d){
	auto p = createAReal();
	arb_set_d(p->val, d);
	return p;
}

LemniAReal lemniCreateARealLong(const LemniInt64 si){
	auto p = createAReal();
	arb_set_si(p->val, si);
	return p;
}

LemniAReal lemniCreateARealULong(const LemniNat64 ui){
	auto p = createAReal();
	arb_set_ui(p->val, ui);
	return p;
}

void lemniDestroyAReal(LemniAReal areal){
	arb_clear(areal->val);
	std::destroy_at(areal);
	std::free(areal);
}

void lemniARealSet(LemniAReal res, LemniARealConst other){
	arb_set(res->val, other->val);
}

void lemniARealStr(LemniARealConst areal, void *user, LemniARealStrCB cb){
	//mpfr_prec_t prec = mpfr_get_prec(areal->val);
	//mpfr_exp_t exp = mpfr_get_exp(areal->val);

	// NOTE: possibly use ARB_STR_NO_RADIUS
	auto str = arb_get_str(areal->val, 12, 0);

	cb(user, LemniStr{.ptr = str, .len = strlen(str)});

	// NOTE: not sure if this is needed
	flint_free(str);

	/*
	mpfr_exp_t exp;
	char *cstr = mpfr_get_str(nullptr, &exp, 10, 0, areal->val, MPFR_RNDN);
	std::string str = cstr;
	str.insert(begin(str) + exp, '.');

	for(std::size_t i = str.size()-1; long(i) > (exp + 1); --i){
		if(str[i] == '0'){
			str.erase(begin(str) + i);
		}
		else{
			break;
		}
	}

	cb(user, {str.c_str(), str.size()});
	mpfr_free_str(cstr);
	*/
}

uint32_t lemniARealNumIntBits(LemniARealConst areal){
	Arb two, z, x;

	arb_set_ui(two, 2);
	arb_floor(x, areal->val, 53);
	arb_log_base_ui(z, x, 2, 53);

	return uint32_t(arf_get_si(arb_midref(z.val), ARF_RND_CEIL));
}

uint32_t lemniARealNumFracBits(LemniARealConst areal){
	return uint32_t(arb_bits(areal->val));
}

bool lemniARealRoundsToFloat(LemniARealConst areal){
	Arf f32Max;

	arf_set_d(f32Max, FLT_MAX);

	bool inRange = arf_cmp(arb_midref(areal->val), f32Max) <= 0;

	if(inRange){
		Arf roundedf, epsilonf;
		Arb rounded, epsilon, diff;

		arf_set_d(epsilonf, FLT_EPSILON);
		arf_set_round(roundedf, arb_midref(areal->val), 23, ARF_RND_NEAR);

		arb_set_arf(rounded, roundedf);
		arb_set_arf(epsilon, epsilonf);
		arb_sub(diff, areal->val, rounded, 53);

		bool sameRounded = arb_lt(diff, epsilon);

		return sameRounded;
	}
	else{
		return false;
	}
}

bool lemniARealRoundsToDouble(LemniARealConst areal){
	Arf f64Max;

	arf_set_d(f64Max, DBL_MAX);

	return arf_cmp(arb_midref(areal->val), f64Max) <= 0;
}

float lemniARealToFloat(LemniARealConst areal){
	return float(lemniARealToDouble(areal));
}

double lemniARealToDouble(LemniARealConst areal){
	return arf_get_d(arb_midref(areal->val), ARF_RND_NEAR);
}

void lemniARealAdd(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	arb_add(res->val, lhs->val, rhs->val, 53);
}

void lemniARealSub(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	arb_sub(res->val, lhs->val, rhs->val, 53);
}

void lemniARealMul(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	arb_mul(res->val, lhs->val, rhs->val, 53);
}

void lemniARealDiv(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs){
	arb_div(res->val, lhs->val, rhs->val, 53);
}

void lemniARealPow(LemniAReal res, LemniARealConst base, LemniARealConst exp){
	arb_pow(res->val, base->val, exp->val, 53);
}

void lemniARealNeg(LemniAReal res, LemniARealConst val){
	arb_neg(res->val, val->val);
}

void lemniARealAbs(LemniAReal res, LemniARealConst val){
	arb_abs(res->val, val->val);
}

int lemniARealCmp(LemniARealConst lhs, LemniARealConst rhs){
	return arf_cmp(arb_midref(lhs->val), arb_midref(rhs->val));
	/*
	Arb z;
	arb_sub(z, lhs->val, rhs->val, 53);
	return arb_sgn_nonzero(z);
	*/
}
