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
#include <cfloat>

#include <new>
#include <memory>
#include <vector>

#include "fmt/format.h"

#include "utf8.h"

#include "lemni/typecheck.h"

#include "Expr.hpp"
#include "TypedExpr.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

struct LemniTypecheckStateT{
	LemniTypeSet types;
	LemniScope globalScope;

	std::vector<std::unique_ptr<LemniExprT>> exprs;
	std::vector<std::unique_ptr<LemniTypedExprT>> typedExprs;
	std::vector<std::unique_ptr<std::string>> errStrs;
	std::map<LemniLValueExpr, LemniTypedExpr> bindings;
	std::map<LemniLValueExpr, LemniTypedLiteralExpr> literalBindings;
};

LemniTypecheckState lemniCreateTypecheckState(LemniTypeSet types){
	auto mem = std::malloc(sizeof(LemniTypecheckStateT));
	auto p = new(mem) LemniTypecheckStateT;
	p->types = types;
	p->globalScope = lemniCreateScope(nullptr);
	return p;
}

void lemniDestroyTypecheckState(LemniTypecheckState state){
	lemniDestroyScope(state->globalScope);
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
		state->typedExprs.emplace_back(std::move(ptr));
		return ret;
	}
}

LemniTypedExtFnDeclExpr lemniCreateTypedExtFn(
	LemniTypecheckState state,
	const LemniStr name, void *const ptr,
	const LemniType resultType,
	const LemniNat64 numParams,
	const LemniType *const paramTypes,
	const LemniStr *const paramNames
){
	std::vector<std::pair<std::string, LemniType>> paramsVec;
	paramsVec.reserve(numParams);

	for(LemniNat64 i = 0; i < numParams; i++){
		paramsVec.emplace_back(lemni::toStdStr(paramNames[i]), paramTypes[i]);
	}

	auto expr = createTypedExpr<LemniTypedExtFnDeclExprT>(state, state->types, lemni::toStdStr(name), ptr, resultType, std::move(paramsVec));

	lemniScopeSet(state->globalScope, expr);

	return expr;
}

LemniTypecheckResult LemniApplicationExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto ref = dynamic_cast<LemniRefExpr>(fn);
	if(ref && (ref->id == "import")){
		if(args.size() != 1){
			return makeError(state, ref->loc, "import expects a single static string argument");
		}

		auto argRes = args[0]->typecheck(state, scope);
		if(argRes.hasError) return argRes;

		auto strExpr = dynamic_cast<LemniTypedStringExpr>(argRes.expr);
		if(!strExpr){
			return makeError(state, args[0]->loc, "import expects a static string argument");
		}

		auto modRes = lemniLoadModule(state->types, lemni::fromStdStrView(strExpr->str()));
		switch(modRes.resType){
			case LEMNI_MODULE_RESULT_LEX:{
				return makeError(
					state, loc,
					fmt::format(
						"lexing error importing module: @{}.{}: {}",
						modRes.lexErr.loc.line, modRes.lexErr.loc.col,
						lemni::toStdStrView(modRes.lexErr.msg)
					)
				);
			}
			case LEMNI_MODULE_RESULT_PARSE:{
				return makeError(
					state, loc,
					fmt::format(
						"parsing error importing module: @{}.{}: {}",
						modRes.parseErr.loc.line, modRes.parseErr.loc.col,
						lemni::toStdStrView(modRes.parseErr.msg)
					)
				);
			}
			case LEMNI_MODULE_RESULT_TYPECHECK:{
				return makeError(
					state, loc,
					fmt::format(
						"typechecking error importing module: @{}.{}: {}",
						modRes.typeErr.loc.line, modRes.typeErr.loc.col,
						lemni::toStdStrView(modRes.typeErr.msg)
					)
				);
			}
			default: break;
		}

		auto moduleType = lemniTypeSetGetModule(state->types);

		auto importExpr = createTypedExpr<LemniTypedImportExprT>(state, moduleType, modRes.module);

		return makeResult(importExpr);
	}

	auto fnRes = fn->typecheck(state, scope);
	if(fnRes.hasError) return fnRes;

	auto fnExprType = fnRes.expr->type();

	if(auto fnType = lemniTypeAsFunction(fnExprType)){
		auto numParams = lemniFunctionTypeNumParams(fnType);
		if(args.size() != numParams){
			auto errStr = "wrong number of arguments passed to function"s;
			if(args.size() < numParams)
				errStr += " (partial application unimplemented)";

			return makeError(state, loc, std::move(errStr));
		}

		std::vector<LemniTypedExpr> argExprs;
		argExprs.reserve(args.size());

		for(decltype(numParams) i = 0; i < numParams; i++){
			auto paramType = lemniFunctionTypeParam(fnType, i);

			auto argRes = args[i]->typecheck(state, scope);
			if(argRes.hasError) return argRes;

			auto argType = argRes.expr->type();

			if(!argType->isCastable(paramType)){
				auto errStr = fmt::format(
					"can not cast argument {} from `{}` to `{}`",
					i + 1, argType->str, paramType->str
				);

				return makeError(state, loc, std::move(errStr));
			}

			argExprs.emplace_back(argRes.expr);
		}

		auto appExpr = createTypedExpr<LemniTypedApplicationExprT>(state, fnType->result, fnRes.expr, std::move(argExprs));
		return makeResult(appExpr);
	}
	else if(auto pseudoType = lemniTypeAsPseudo(fnExprType)){
		std::vector<LemniTypedExpr> argExprs;
		argExprs.reserve(args.size());

		for(auto arg : args){
			auto argRes = arg->typecheck(state, scope);
			if(argRes.hasError) return argRes;

			argExprs.emplace_back(argRes.expr);
		}

		auto resultType = lemniTypeSetGetPseudo(state->types, lemniEmptyTypeInfo());
		auto appExpr = createTypedExpr<LemniTypedApplicationExprT>(state, resultType, fnRes.expr, std::move(argExprs));
		return makeResult(appExpr);
	}
	else{
		return makeError(state, loc, "application on non-function expression");
	}
}

LemniTypecheckResult LemniTupleExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	if(elements.empty()){
		return makeResult(createTypedExpr<LemniTypedUnitExprT>(state, lemniTypeSetGetUnit(state->types)));
	}

	std::vector<LemniTypedExpr> elems;
	std::vector<LemniType> elemTypes;
	elems.reserve(elements.size());
	elemTypes.reserve(elements.size());

	for(auto elem : elements){
		auto res = elem->typecheck(state, scope);
		if(res.hasError)
			return res;
		else{
			elems.emplace_back(res.expr);
			elemTypes.emplace_back(res.expr->type());
		}
	}

	return makeResult(createTypedExpr<LemniTypedProductExprT>(state, state->types, std::move(elems)));
}

LemniTypecheckResult LemniUnitExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	auto unitType = lemniTypeSetGetUnit(state->types);
	return makeResult(createTypedExpr<LemniTypedUnitExprT>(state, unitType));
}

LemniTypecheckResult LemniRealExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	if(val.roundsToFloat()){
		auto fltType = lemniTypeSetGetReal(state->types, 32);
		auto fltExpr = createTypedExpr<LemniTypedReal32ExprT>(state, fltType, val.toFloat());
		return makeResult(fltExpr);
	}
	else if(val.roundsToDouble()){
		// TODO: emit warning
		// lemniWarn("arbitrary-precision real numbers are currently rounded to double precision unless x > DBL_MAX");
		auto dblType = lemniTypeSetGetReal(state->types, 64);
		auto dblExpr = createTypedExpr<LemniTypedReal64ExprT>(state, dblType, val.toDouble());
		return makeResult(dblExpr);
	}
	else{
		auto realType = lemniTypeSetGetReal(state->types, 0);
		return makeResult(createTypedExpr<LemniTypedARealExprT>(state, realType, val));
	}
}

LemniTypecheckResult LemniRatioExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	auto numBits = val.numBits();
	auto maxNumBits = ceilPowerOfTwo(std::max(numBits.num, numBits.den)) * 2;

	if(maxNumBits < 32)
		maxNumBits = 32;
	else if(maxNumBits > 128)
		maxNumBits = 0;

	auto ratioType = lemniTypeSetGetRatio(state->types, maxNumBits);

	LemniTypedRatioExpr ratioExpr;

	switch(maxNumBits){
		case 32:{
			auto ratio128 = val.toRatio128();
			ratioExpr = createTypedExpr<LemniTypedRatio32ExprT>(
				state,
				ratioType,
				LemniRatio32{int16_t(ratio128.num), uint16_t(ratio128.den)}
			);
			break;
		}

		case 64:{
			auto ratio128 = val.toRatio128();
			ratioExpr = createTypedExpr<LemniTypedRatio64ExprT>(
				state,
				ratioType,
				LemniRatio64{int32_t(ratio128.num), uint32_t(ratio128.den)}
			);
			break;
		}

		case 128:{
			ratioExpr = createTypedExpr<LemniTypedRatio128ExprT>(state, ratioType, val.toRatio128());
			break;
		}

		case 0:
		default:{
			ratioExpr = createTypedExpr<LemniTypedARatioExprT>(state, ratioType, val);
			break;
		}
	}

	return makeResult(ratioExpr);
}

LemniTypecheckResult LemniIntExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	if(val < lemni::AInt(0)){
		// integer
		auto numBits = ceilPowerOfTwo(val.numBits());
		if(numBits < 16)
			numBits = 16;
		else if(numBits > 64)
			numBits = 0;

		auto intType = lemniTypeSetGetInt(state->types, numBits);

		LemniTypedIntExpr intExpr;

		switch(numBits){
			case 16:{
				intExpr = createTypedExpr<LemniTypedInt16ExprT>(state, intType, val.toLong());
				break;
			}

			case 32:{
				intExpr = createTypedExpr<LemniTypedInt32ExprT>(state, intType, val.toLong());
				break;
			}

			case 64:{
				intExpr = createTypedExpr<LemniTypedInt64ExprT>(state, intType, val.toLong());
				break;
			}

			case 0:
			default:{
				intExpr = createTypedExpr<LemniTypedAIntExprT>(state, intType, val);
				break;
			}
		}

		return makeResult(intExpr);
	}
	else{
		// natural
		auto numBits = ceilPowerOfTwo(val.numBitsUnsigned());
		if(numBits < 16)
			numBits = 16;
		else if(numBits > 64)
			numBits = 0;

		auto natType = lemniTypeSetGetNat(state->types, numBits);

		LemniTypedNatExpr natExpr;

		switch(numBits){
			case 16:{
				natExpr = createTypedExpr<LemniTypedNat16ExprT>(state, natType, val.toULong());
				break;
			}

			case 32:{
				natExpr = createTypedExpr<LemniTypedNat32ExprT>(state, natType, val.toULong());
				break;
			}

			case 64:{
				natExpr = createTypedExpr<LemniTypedNat64ExprT>(state, natType, val.toULong());
				break;
			}

			case 0:
			default:{
				natExpr = createTypedExpr<LemniTypedANatExprT>(state, natType, val);
				break;
			}
		}

		return makeResult(natExpr);
	}
}

LemniTypecheckResult LemniStrExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	auto strBeg = begin(val);
	auto strEnd = end(val);
	auto strIt = strBeg;

	bool isUtf8 = false;

	while(strIt != strEnd){
		auto cp = utf8::next(strIt, strEnd);
		if(cp > 127){
			// non-ascii code
			isUtf8 = true;
			break;
		}
	}

	LemniTypedExpr strExpr = nullptr;

	if(isUtf8){
		auto strType = lemniTypeSetGetStringUTF8(state->types);
		strExpr = createTypedExpr<LemniTypedStringUTF8ExprT>(state, strType, val);
	}
	else{
		auto strType = lemniTypeSetGetStringASCII(state->types);
		strExpr = createTypedExpr<LemniTypedStringASCIIExprT>(state, strType, val);
	}

	return makeResult(strExpr);
}

LemniTypecheckResult LemniCommaListExprT::typecheck(LemniTypecheckState state, LemniScope) const noexcept{
	return makeError(state, this->loc, "implementation type 'comma-list-expr' should not be typechecked directly");
}

LemniTypecheckResult LemniUnaryOpExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto valueChecked = expr->typecheck(state, scope);

	if(valueChecked.hasError)
		return valueChecked;

	auto resultType = lemniUnaryOpResultType(state->types, valueChecked.expr->type(), op);

	if(!resultType){
		return makeError(state, loc, "unary operation undefined on value type");
	}

	auto resultExpr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resultType, op, valueChecked.expr);

	return makeResult(resultExpr);
}

LemniTypecheckResult LemniBinaryOpExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto lhsChecked = lhs->typecheck(state, scope);
	if(lhsChecked.hasError)
		return lhsChecked;

	auto rhsChecked = rhs->typecheck(state, scope);
	if(rhsChecked.hasError)
		return rhsChecked;

	auto resultType = lemniBinaryOpResultType(state->types, lhsChecked.expr->type(), rhsChecked.expr->type(), op);
	if(!resultType){
		return makeError(state, loc, "binary operation undefined on value types");
	}

	auto resultExpr = createTypedExpr<LemniTypedBinaryOpExprT>(state, resultType, op, lhsChecked.expr, rhsChecked.expr);

	return makeResult(resultExpr);
}

LemniTypecheckResult LemniLambdaExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	std::vector<LemniTypedExpr> typedParams;
	std::vector<LemniType> paramTypes;
	typedParams.reserve(params.size());
	paramTypes.reserve(params.size());

	for(auto param : params){
		auto res = param->typecheck(state, scope);
		if(res.hasError)
			return res;
		else{
			typedParams.emplace_back(res.expr);
			paramTypes.emplace_back(res.expr->type());
		}
	}

	auto typedBodyRes = body->typecheck(state, scope);
	if(typedBodyRes.hasError)
		return typedBodyRes;

	auto fnType = lemniTypeSetGetFunction(state->types, typedBodyRes.expr->type(), paramTypes.data(), paramTypes.size());

	auto lamExpr = createTypedExpr<LemniTypedLambdaExprT>(state, fnType, std::move(typedParams), typedBodyRes.expr);

	return makeResult(lamExpr);
}

LemniTypecheckResult LemniBranchExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto condRes = cond->typecheck(state, scope);
	if(condRes.hasError) return condRes;

	auto trueRes = true_->typecheck(state, scope);
	if(trueRes.hasError) return trueRes;

	auto falseRes = false_->typecheck(state, scope);
	if(falseRes.hasError) return falseRes;

	auto resultType = lemniTypePromote(state->types, trueRes.expr->type(), falseRes.expr->type());

	auto branchExpr = createTypedExpr<LemniTypedBranchExprT>(state, resultType, condRes.expr, trueRes.expr, falseRes.expr);

	return makeResult(branchExpr);
}

namespace {
	inline LemniType findReturnExprType(LemniTypeSet types, LemniScope scope, LemniTypedExpr expr){
		if(auto ret = dynamic_cast<LemniTypedReturnExpr>(expr)){
			return ret->value->type();
		}
		else if(auto branch = dynamic_cast<LemniTypedBranchExpr>(expr)){
			auto trueRet = findReturnExprType(types, scope, branch->true_);
			auto falseRet = findReturnExprType(types, scope, branch->false_);

			if(trueRet && falseRet){
				return lemniTypePromote(types, trueRet, falseRet);
			}
			else if(trueRet){
				return trueRet;
			}
			else if(falseRet){
				return falseRet;
			}
		}

		return nullptr;
	}
}

LemniTypecheckResult LemniBlockExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	std::vector<LemniTypedExpr> typedExprs;
	std::vector<LemniType> returnTypes;
	typedExprs.reserve(exprs.size());

	auto innerScope = lemni::Scope(scope);

	for(auto expr : exprs){
		auto res = expr->typecheck(state, innerScope);
		if(res.hasError) return res;
		else typedExprs.emplace_back(res.expr);
	}

	for(auto it = begin(typedExprs); it != end(typedExprs)-1; ++it){
		if(auto retType = findReturnExprType(state->types, innerScope, *it))
			returnTypes.emplace_back(retType);
	}

	returnTypes.emplace_back(typedExprs.back()->type());

	auto retType = returnTypes.front();

	for(auto it = begin(returnTypes)+1; it != end(returnTypes); ++it){
		retType = lemniTypePromote(state->types, retType, *it);
	}

	auto blockExpr = createTypedExpr<LemniTypedBlockExprT>(state, retType, std::move(typedExprs));

	return makeResult(blockExpr);
}

LemniTypecheckResult LemniRefExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	if(id == "true"){
		auto trueExpr = createTypedExpr<LemniTypedBoolExprT>(state, lemniTypeSetGetBool(state->types), true);
		return makeResult(trueExpr);
	}
	else if(id == "false"){
		auto falseExpr = createTypedExpr<LemniTypedBoolExprT>(state, lemniTypeSetGetBool(state->types), false);
		return makeResult(falseExpr);
	}

	auto res = lemniScopeFind(scope, lemni::fromStdStrView(id));
	if(res){
		auto refExpr = createTypedExpr<LemniTypedRefExprT>(state, res);
		return makeResult(refExpr);
	}
	else{
		auto unresolvedExpr = createTypedExpr<LemniTypedUnresolvedRefExprT>(state, id, lemniTypeSetGetPseudo(state->types, lemniEmptyTypeInfo()));
		return makeResult(unresolvedExpr);
	}
}

LemniTypecheckResult LemniBindingExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto valRes = value->typecheck(state, scope);
	if(valRes.hasError) return valRes;

	auto bindingExpr = createTypedExpr<LemniTypedBindingExprT>(state, this->id, valRes.expr);

	lemniScopeSet(scope, bindingExpr);

	return makeResult(bindingExpr);
}

LemniTypecheckResult LemniParamBindingExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	if(type){
		auto typeRes = type->typecheck(state, scope);
		if(typeRes.hasError) return typeRes;

		if(typeRes.expr->type() != lemniTypeSetGetMeta(state->types)){
			return makeError(state, this->loc, "expression given for parameter type is not a type expression");
		}

		auto const_ = dynamic_cast<LemniTypedTypeExpr>(typeRes.expr);
		if(!const_){
			return makeError(state, this->loc, "only constant type expressions are currently supported");
		}

		auto typedParam = createTypedExpr<LemniTypedParamBindingExprT>(state, this->id, const_->value);

		lemniScopeSet(scope, typedParam);

		return makeResult(typedParam);
	}
	else{
		LemniTypeInfo usageInfo;
		usageInfo.binaryOpFlags = 0;
		usageInfo.unaryOpFlags = 0;
		usageInfo.typeClass = LEMNI_TYPECLASS_EMPTY;
		std::memset(usageInfo.info.bytes, 0, sizeof(usageInfo.info.bytes));

		auto pseudoType = lemniTypeSetGetPseudo(state->types, usageInfo);

		auto typedParam = createTypedExpr<LemniTypedParamBindingExprT>(state, this->id, pseudoType);

		lemniScopeSet(scope, typedParam);

		return makeResult(typedParam);
	}
}

LemniTypecheckResult LemniFnDefExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	std::vector<LemniTypedParamBindingExpr> typedParams;
	typedParams.reserve(params.size());

	auto innerScope = lemni::Scope(scope);

	for(auto param : params){
		auto paramRes = param->typecheck(state, innerScope);
		if(paramRes.hasError) return paramRes;

		auto typedBinding = dynamic_cast<LemniTypedParamBindingExpr>(paramRes.expr);
		if(!typedBinding){
			LemniTypecheckResult ret;
			ret.hasError = true;
			ret.error.loc = param->loc;
			ret.error.msg = LEMNICSTR("only name parameters currently supported");
			return ret;
		}

		typedParams.emplace_back(typedBinding);
	}

	auto bodyRes = body->typecheck(state, innerScope);
	if(bodyRes.hasError) return bodyRes;

	if(params.size() == 0){
		auto unitArg = createTypedExpr<LemniTypedParamBindingExprT>(state, "$0", lemniTypeSetGetUnit(state->types));
		typedParams.emplace_back(unitArg);
	}

	auto fnDefExpr = createTypedExpr<LemniTypedFnDefExprT>(state, state->types, id, bodyRes.expr->type(), std::move(typedParams), bodyRes.expr);

	lemniScopeSet(scope, fnDefExpr);

	return makeResult(fnDefExpr);
}

LemniTypecheckResult lemniTypecheck(LemniTypecheckState state, LemniExpr expr){
	if(!expr){
		return makeResult(nullptr);
	}

	return expr->typecheck(state, state->globalScope);
}

LemniType lemniUnaryOpResultType(LemniTypeSet types, LemniType value, LemniUnaryOp op){
	auto typeInfo = lemniTypeSetGetTypeInfo(types, value);

	if(lemniTypeAsBool(value)){
		if(op != LEMNI_UNARY_NOT)
			return nullptr;

		return value;
	}
	else if(lemniTypeInfoIsArithmetic(typeInfo)){
		if(op != LEMNI_UNARY_NEG)
			return nullptr;

		return lemniTypeMakeSigned(types, value);
	}

	return nullptr;
}

LemniType lemniBinaryOpResultType(LemniTypeSet types, LemniType lhs, LemniType rhs, LemniBinaryOp op){
	auto lhsInfo = lemniTypeSetGetTypeInfo(types, lhs);
	auto rhsInfo = lemniTypeSetGetTypeInfo(types, rhs);

	if(lemniTypeAsPseudo(lhs) || lemniTypeAsPseudo(rhs)){
		// TODO: or usage flags
		return lemniTypeSetGetPseudo(types, lemniEmptyTypeInfo());
	}
	else if(lemniTypeInfoIsArithmetic(lhsInfo) && lemniTypeInfoIsArithmetic(rhsInfo)){
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
