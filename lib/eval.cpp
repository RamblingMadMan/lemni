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

#include <vector>
#include <string>
#include <memory>
#include <new>

#include "lemni/eval.h"

#include "TypedExpr.hpp"

struct LemniEvalStateT{
	std::vector<std::string> errMsgs;
};

LemniEvalState lemniCreateEvalState(void){
	auto mem = std::malloc(sizeof(LemniEvalStateT));
	auto p = new(mem) LemniEvalStateT;
	return p;
}

void lemniDestroyEvalState(LemniEvalState state){
	std::destroy_at(state);
	std::free(state);
}

LemniEvalResult lemniEval(LemniEvalState state, LemniTypedExpr expr){
	LemniEvalResult res;

	if(auto constExpr = dynamic_cast<LemniTypedConstantExpr>(expr)){
		if(auto natExpr = dynamic_cast<LemniTypedNatExpr>(constExpr)){
			res.hasError = false;

			auto n = natExpr->value.numBitsUnsigned();

			if(n > 64){
				res.value = lemniCreateValueANat(natExpr->value.handle());
			}
			else{
				auto val = lemniAIntToULong(natExpr->value.handle());

				if(n <= 16){
					res.value = lemniCreateValueNat16(static_cast<LemniNat16>(val));
				}
				else if(n <= 32){
					res.value = lemniCreateValueNat32(static_cast<LemniNat32>(val));
				}
				else{
					res.value = lemniCreateValueNat64(static_cast<LemniNat64>(val));
				}
			}
		}
		else if(auto intExpr = dynamic_cast<LemniTypedIntExpr>(constExpr)){
			res.hasError = false;

			auto n = intExpr->value.numBits();

			if(n > 64){
				res.value = lemniCreateValueAInt(intExpr->value.handle());
			}
			else{
				auto val = lemniAIntToLong(natExpr->value.handle());

				if(n <= 16){
					res.value = lemniCreateValueInt16(static_cast<LemniInt16>(val));
				}
				else if(n <= 32){
					res.value = lemniCreateValueInt32(static_cast<LemniInt32>(val));
				}
				else{
					res.value = lemniCreateValueInt64(static_cast<LemniInt64>(val));
				}
			}
		}
		else if(auto ratioExpr = dynamic_cast<LemniTypedRatioExpr>(constExpr)){
			res.hasError = false;

			auto bitPair = ratioExpr->value.numBits();
			auto n = std::max(bitPair.num, bitPair.den);

			if(n > 64){
				res.value = lemniCreateValueARatio(ratioExpr->value.handle());
			}
			else{
				auto num = ratioExpr->value.num().toLong();
				auto den = ratioExpr->value.den().toULong();

				if(n <= 16){
					res.value = lemniCreateValueRatio32({static_cast<LemniInt16>(num), static_cast<LemniNat16>(den)});
				}
				else if(n <= 32){
					res.value = lemniCreateValueRatio64({static_cast<LemniInt32>(num), static_cast<LemniNat32>(den)});
				}
				else{
					res.value = lemniCreateValueRatio128({static_cast<LemniInt64>(num), static_cast<LemniNat64>(den)});
				}
			}
		}
		else if(auto realExpr = dynamic_cast<LemniTypedRealExpr>(constExpr)){
			res.hasError = false;
			res.value = lemniCreateValueAReal(realExpr->value.handle());
		}
		else{
			auto &&msg = state->errMsgs.emplace_back("unrecognized constant expression");
			res.hasError = true;
			res.error = {msg.c_str(), msg.size()};
		}
	}
	else if(auto binopExpr = dynamic_cast<LemniTypedBinaryOpExpr>(expr)){

	}
	else{
		auto &&msg = state->errMsgs.emplace_back("unrecognized expression");
		res.hasError = true;
		res.error = {msg.c_str(), msg.size()};
	}

	return res;
}
