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
#include <vector>

#include "lemni/typecheck.h"

#include "Expr.hpp"
#include "TypedExpr.hpp"

struct LemniTypecheckStateT{
	LemniTypeSet types;
	const LemniExpr *exprs;
	size_t numExprs;

	std::vector<std::unique_ptr<LemniTypedExprT>> typedExprs;
	std::vector<std::unique_ptr<std::string>> errStrs;
};

LemniTypecheckState lemniCreateTypecheckState(LemniTypeSet types, const LemniExpr *const exprs, const size_t numExprs){
	auto mem = std::malloc(sizeof(LemniTypecheckStateT));
	auto p = new(mem) LemniTypecheckStateT;
	p->types = types;
	p->exprs = exprs;
	p->numExprs = numExprs;
	return p;
}

void lemniDestroyTypecheckState(LemniTypecheckState state){
	std::destroy_at(state);
	std::free(state);
}

namespace {
	inline LemniTypecheckResult makeError(LemniTypecheckState state, LemniLocation loc, std::string msg){
/*
#ifndef NDEBUG
		throw std::runtime_error(msg);
#endif
*/
		auto &&str = state->errStrs.emplace_back(std::make_unique<std::string>(std::move(msg)));
		LemniTypecheckResult ret;
		ret.hasError = true;
		ret.error = { .loc = loc, .msg = { .ptr = str->c_str(), .len = str->size() } };
		return ret;
	}

	inline LemniTypecheckResult makeResult(LemniTypedExpr expr){
		LemniTypecheckResult ret;
		ret.hasError = false;
		ret.expr = expr;
		return ret;
	}

	template<typename T, typename ... Args>
	inline T *createTypedExpr(LemniTypecheckState state, Args &&... args){
		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		auto ret = ptr.get();
		auto insertIt = std::lower_bound(cbegin(state->typedExprs), cend(state->typedExprs), ptr);
		state->typedExprs.insert(insertIt, std::move(ptr));
		return ret;
	}

	LemniTypecheckResult typecheckNumber(LemniTypecheckState state, const LemniExpr *it, const LemniExpr *const end, LemniNumExpr num){
		if(auto int_ = lemniNumExprAsInt(num)){
			auto val = lemni::AInt::from(lemniCreateAIntCopy(int_->val));
			auto zero = lemni::AInt(0);

			state->exprs = it;
			state->numExprs = static_cast<size_t>(std::distance(it, end));

			if(val < zero){
				// integer
				auto intType = lemniTypeSetGetInt(state->types, val.numBits());
				auto intExpr = createTypedExpr<LemniTypedIntExprT>(state, intType, std::move(val));
				return makeResult(intExpr);
			}
			else{
				// natural
				auto natType = lemniTypeSetGetNat(state->types, val.numBitsUnsigned());
				auto natExpr = createTypedExpr<LemniTypedNatExprT>(state, natType, std::move(val));
				return makeResult(natExpr);
			}
		}
		else if(auto ratio = lemniNumExprAsRatio(num)){
			auto val = lemni::ARatio::from(lemniCreateARatioCopy(ratio->val));
			auto numBits = val.numBits();
			auto maxNumBits = std::max(numBits.num, numBits.den);
			auto ratioType = lemniTypeSetGetRatio(state->types, maxNumBits * 2);
			auto ratioExpr = createTypedExpr<LemniTypedRatioExprT>(state, ratioType, std::move(val));

			state->exprs = it;
			state->numExprs = static_cast<size_t>(std::distance(it, end));

			return makeResult(ratioExpr);

		}
		else if(auto real = lemniNumExprAsReal(num)){
			auto constant = lemniNumExprBase(num);
			auto lit = lemniConstantExprBase(constant);
			return makeError(state, lemniExprLoc(lemniLiteralExprBase(lit)), "Typechecking unimplemented for real literals");
		}
		else{
			auto constant = lemniNumExprBase(num);
			auto lit = lemniConstantExprBase(constant);
			return makeError(state, lemniExprLoc(lemniLiteralExprBase(lit)), "Typechecking unimplemented for number expression");
		}
	}

	LemniTypecheckResult typecheckLiteral(LemniTypecheckState state, const LemniExpr *it, const LemniExpr *const end, LemniLiteralExpr lit){
		if(auto constant = lemniLiteralExprAsConstant(lit)){
			if(auto num = lemniConstantExprAsNum(constant)){
				return typecheckNumber(state, it, end, num);
			}
			else{
				return makeError(state, lemniExprLoc(lemniLiteralExprBase(lit)), "Typechecking unimplemented for constant expression");
			}
		}
		else{
			return makeError(state, lemniExprLoc(lemniLiteralExprBase(lit)), "Typechecking unimplemented for expression");
		}
	}

	LemniTypecheckResult typecheckInner(LemniTypecheckState state, const LemniExpr *it, const LemniExpr *const end){
		auto expr = *it;

		if(auto lit = lemniExprAsLiteral(expr)){
			return typecheckLiteral(state, ++it, end, lit);
		}

		else{
			return makeError(state, lemniExprLoc(expr), "Typechecking unimplemented for expression");
		}
	}
}

LemniTypecheckResult lemniTypecheck(LemniTypecheckState state){
	auto it = state->exprs;
	auto end = it + state->numExprs;

	if(it == end){
		return makeResult(nullptr);
	}

	return typecheckInner(state, it, end);
}

LemniType lemniBinaryOpResultType(LemniTypeSet types, LemniType lhs, LemniType rhs, LemniBinaryOp op){
	if(lemniTypeIsArithmetic(lhs) && lemniTypeIsArithmetic(rhs)){
		if(lemniBinaryOpIsLogic(op))
			return lemniTypeSetGetBool(types);
		else if(op == LEMNI_BINARY_SUB){
			lhs = lemniTypeMakeSigned(types, lhs);
			rhs = lemniTypeMakeSigned(types, rhs);
		}

		return lemniTypePromote(types, lhs, rhs);
	}

	// TODO: implement sum type results

	return nullptr;
}
