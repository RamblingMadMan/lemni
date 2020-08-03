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
	~LemniTypecheckStateT(){
		for(auto expr : alloced){
			deleteTypedExpr(expr);
		}
	}

	LemniModuleMap mods;
	LemniTypeSet types;
	LemniScope globalScope;

	//std::vector<std::unique_ptr<LemniExprT>> exprs;
	std::vector<LemniTypedExpr> alloced;
	std::vector<std::unique_ptr<std::string>> errStrs;
	//std::map<LemniLValueExpr, LemniTypedExpr> bindings;
	//std::map<LemniLValueExpr, LemniTypedLiteralExpr> literalBindings;
};

LemniTypecheckState lemniCreateTypecheckState(LemniModuleMap mods){
	auto mem = std::malloc(sizeof(LemniTypecheckStateT));
	auto p = new(mem) LemniTypecheckStateT;
	p->mods = mods;
	p->types = lemniModuleMapTypes(mods);
	p->globalScope = lemniCreateScope(nullptr);
	return p;
}

void lemniDestroyTypecheckState(LemniTypecheckState state){
	lemniDestroyScope(state->globalScope);
	std::destroy_at(state);
	std::free(state);
}

LemniModuleMap lemniTypecheckStateModuleMap(LemniTypecheckState state){
	return state->mods;
}

LemniScope lemniTypecheckStateScope(LemniTypecheckStateConst state){
	return state->globalScope;
}

LemniTypecheckResult lemniTypecheckEval(LemniTypecheckState state, LemniTypedExpr expr, const LemniNat64 numArgs, LemniTypedExpr *const args){
	auto nullBindings = LemniPartialBindingsT();
	return expr->partialEval(state, &nullBindings, numArgs, args);
}

namespace {
	inline LemniTypecheckResult litError(LemniLocation loc, LemniStr str){
		LemniTypecheckResult ret;
		ret.hasError = true;
		ret.error = { .loc = loc, .msg = str };
		return ret;
	}

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
		auto p = newTypedExpr<T>(std::forward<Args>(args)...);
		if(!p) return nullptr;

		auto res = std::upper_bound(begin(state->alloced), end(state->alloced), p);
		state->alloced.insert(res, p);

		return p;
	}

	template<typename ... Fs> struct Overload: Fs...{ using Fs::operator()...; };
	template<typename ... Fs> Overload(Fs...) -> Overload<Fs...>;

	struct NaturalAInt{
		LemniAInt value;
	};

	namespace detail{
		template<typename...> struct Tag;

		template<typename T, typename = void> struct ValueTyped;

		template<>
		struct ValueTyped<NaturalAInt>{
			static LemniTypedANatExpr create(LemniTypecheckState state, NaturalAInt val) noexcept{
				return createTypedExpr<LemniTypedANatExprT>(state, lemniTypeSetGetNat(state->types, 0), lemni::AInt::from(val.value));
			}
		};

		template<>
		struct ValueTyped<lemni::AInt>{
			static LemniTypedAIntExpr create(LemniTypecheckState state, lemni::AInt val) noexcept{
				return createTypedExpr<LemniTypedAIntExprT>(state, lemniTypeSetGetInt(state->types, 0), std::move(val));
			}
		};

		template<>
		struct ValueTyped<lemni::ARatio>{
			static LemniTypedARatioExpr create(LemniTypecheckState state, lemni::ARatio val) noexcept{
				return createTypedExpr<LemniTypedARatioExprT>(state, lemniTypeSetGetRatio(state->types, 0), std::move(val));
			}
		};

		template<>
		struct ValueTyped<lemni::AReal>{
			static LemniTypedARealExpr create(LemniTypecheckState state, lemni::AReal val) noexcept{
				return createTypedExpr<LemniTypedARealExprT>(state, lemniTypeSetGetReal(state->types, 0), std::move(val));
			}
		};

		template<typename Nat>
		struct ValueTyped<Nat, std::enable_if_t<lemni::is_natural_v<Nat>>>{
			using Expr = std::conditional_t<
					std::is_same_v<Nat, LemniNat16>, LemniTypedNat16ExprT, std::conditional_t<
					std::is_same_v<Nat, LemniNat32>, LemniTypedNat32ExprT, std::conditional_t<
					std::is_same_v<Nat, LemniNat64>, LemniTypedNat64ExprT, void
				>>>;

			static const Expr *create(LemniTypecheckState state, const Nat val) noexcept{
				return createTypedExpr<Expr>(state, lemniTypeSetGetNat(state->types, sizeof(Nat) * CHAR_BIT), val);
			}
		};

		template<typename Int>
		struct ValueTyped<Int, std::enable_if_t<lemni::is_integer_v<Int>>>{
			using Expr = std::conditional_t<
					std::is_same_v<Int, LemniInt16>, LemniTypedInt16ExprT, std::conditional_t<
					std::is_same_v<Int, LemniInt32>, LemniTypedInt32ExprT, std::conditional_t<
					std::is_same_v<Int, LemniInt64>, LemniTypedInt64ExprT, void
				>>>;

			static const Expr *create(LemniTypecheckState state, const Int val) noexcept{
				return createTypedExpr<Expr>(state, lemniTypeSetGetInt(state->types, sizeof(Int) * CHAR_BIT), val);
			}
		};

		template<typename Ratio>
		struct ValueTyped<Ratio, std::enable_if_t<lemni::is_ratio_v<Ratio>>>{
			using Expr = std::conditional_t<
					std::is_same_v<Ratio, LemniRatio32>, LemniTypedRatio32ExprT, std::conditional_t<
					std::is_same_v<Ratio, LemniRatio64>, LemniTypedRatio64ExprT, std::conditional_t<
					std::is_same_v<Ratio, LemniRatio128>, LemniTypedRatio128ExprT, void
				>>>;

			static const Expr *create(LemniTypecheckState state, const Ratio val) noexcept{
				return createTypedExpr<Expr>(state, lemniTypeSetGetRatio(state->types, sizeof(Ratio) * CHAR_BIT), val);
			}
		};

		template<typename Real>
		struct ValueTyped<Real, std::enable_if_t<lemni::is_real_v<Real>>>{
			using Expr = std::conditional_t<
					std::is_same_v<Real, float>, LemniTypedReal32ExprT, std::conditional_t<
					std::is_same_v<Real, double>, LemniTypedReal64ExprT, void
				>>;

			static const Expr *create(LemniTypecheckState state, const Real val) noexcept{
				return createTypedExpr<Expr>(state, lemniTypeSetGetReal(state->types, sizeof(Real) * CHAR_BIT), val);
			}
		};
	}

	template<typename T>
	inline LemniTypecheckResult typecheckCppLit(LemniTypecheckState state, T val){
		auto expr = detail::ValueTyped<std::remove_cv_t<T>>::create(state, val);
		return makeResult(expr);
	}

	template<typename F, typename ... Args>
	inline auto typedNumApply(F &&f, LemniTypedNumExpr num, Args &&... args){
		static constexpr auto natApply = [](LemniTypedNatExpr nat, auto &&f, auto &&... args){
			if(auto nat16 = dynamic_cast<LemniTypedNat16Expr>(nat)) return f(nat16->value, std::forward<Args>(args)...);
			else if(auto nat32 = dynamic_cast<LemniTypedNat32Expr>(nat)) return f(nat32->value, std::forward<Args>(args)...);
			else if(auto nat64 = dynamic_cast<LemniTypedNat64Expr>(nat)) return f(nat64->value, std::forward<Args>(args)...);
			else if(auto anat = dynamic_cast<LemniTypedANatExpr>(nat)) return f(NaturalAInt{ lemniCreateAIntCopy(anat->value) }, std::forward<Args>(args)...);
			else return f(nat, std::forward<Args>(args)...);
		};

		static constexpr auto intApply = [](LemniTypedIntExpr int_, auto &&f, auto &&... args){
			if(auto int16 = dynamic_cast<LemniTypedInt16Expr>(int_)) return f(int16->value, std::forward<Args>(args)...);
			else if(auto int32 = dynamic_cast<LemniTypedInt32Expr>(int_)) return f(int32->value, std::forward<Args>(args)...);
			else if(auto int64 = dynamic_cast<LemniTypedInt64Expr>(int_)) return f(int64->value, std::forward<Args>(args)...);
			else if(auto aint = dynamic_cast<LemniTypedAIntExpr>(int_)) return f(aint->value, std::forward<Args>(args)...);
			else return f(int_, std::forward<Args>(args)...);
		};

		static constexpr auto ratioApply = [](LemniTypedRatioExpr ratio, auto &&f, auto &&... args){
			if(auto rat32 = dynamic_cast<LemniTypedRatio32Expr>(ratio)) return f(rat32->value, std::forward<Args>(args)...);
			else if(auto rat64 = dynamic_cast<LemniTypedRatio64Expr>(ratio)) return f(rat64->value, std::forward<Args>(args)...);
			else if(auto rat128 = dynamic_cast<LemniTypedRatio128Expr>(ratio)) return f(rat128->value, std::forward<Args>(args)...);
			else if(auto arat = dynamic_cast<LemniTypedARatioExpr>(ratio)) return f(arat->value, std::forward<Args>(args)...);
			else return f(ratio, std::forward<Args>(args)...);
		};

		static constexpr auto realApply = [](LemniTypedRealExpr real, auto &&f, auto &&... args){
			if(auto real32 = dynamic_cast<LemniTypedReal32Expr>(real)) return f(real32->value, std::forward<Args>(args)...);
			else if(auto real64 = dynamic_cast<LemniTypedReal64Expr>(real)) return f(real64->value, std::forward<Args>(args)...);
			else if(auto areal = dynamic_cast<LemniTypedARealExpr>(real)) return f(areal->value, std::forward<Args>(args)...);
			else return f(real, std::forward<Args>(args)...);
		};

		if(auto nat = dynamic_cast<LemniTypedNatExpr>(num)) return natApply(nat, std::forward<F>(f), std::forward<Args>(args)...);
		else if(auto int_ = dynamic_cast<LemniTypedIntExpr>(num)) return intApply(int_, std::forward<F>(f), std::forward<Args>(args)...);
		else if(auto ratio = dynamic_cast<LemniTypedRatioExpr>(num)) return ratioApply(ratio, std::forward<F>(f), std::forward<Args>(args)...);
		else if(auto real = dynamic_cast<LemniTypedRealExpr>(num)) return realApply(real, std::forward<F>(f), std::forward<Args>(args)...);
		else return f(num, std::forward<Args>(args)...);
	}

	/*
	inline LemniTypecheckResult typecheckNumBinopDispatch(LemniTypecheckState state, const LemniBinaryOp op, LemniTypedNumExpr lhs, LemniTypedNumExpr rhs){
		static constexpr auto lhsApplicator = [](auto lhsVal, LemniTypecheckState state, const LemniBinaryOp op, LemniTypedNumExpr rhs){
			static constexpr auto rhsApplicator = [](auto rhsVal, auto lhsVal, LemniTypecheckState state, const LemniBinaryOp op){
				switch(op){
					case LEMNI_BINARY_ADD:{ return lhsVal + rhsVal; }
					case LEMNI_BINARY_SUB:{ return lhsVal - rhsVal; }
					case LEMNI_BINARY_MUL:{ return lhsVal * rhsVal; }
					case LEMNI_BINARY_DIV:{ return lhsVal / rhsVal; }
					case LEMNI_BINARY_MOD:{ return lhsVal % rhsVal; }
					default:{ return makeError(state, LemniLocation{ UINT32_MAX, UINT32_MAX }, "undefined binary operator for numeric types"); }
				}
			};

			return typedNumApply(rhsApplicator, rhs, state, op, lhsVal);
		};

		return typedNumApply(lhsApplicator, lhs, state, op, rhs);
	}
	*/

	inline LemniTypecheckResult typecheckNumUnaryOpDispatch(LemniTypecheckState state, const LemniUnaryOp op, LemniTypedNumExpr num){
		static constexpr auto applicator = [](auto val, LemniTypecheckState state, const LemniUnaryOp op) -> LemniTypecheckResult{
			using Val = std::remove_cv_t<decltype(val)>;

			if constexpr(std::is_base_of_v<LemniTypedExprT, std::remove_pointer_t<Val>>){
				auto resType = lemniUnaryOpResultType(state->types, val->type(), op);
				auto expr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resType, op, val);
				return makeResult(expr);
			}
			else{
				switch(op){
					case LEMNI_UNARY_NEG:{
						if constexpr(std::is_same_v<Val, NaturalAInt>){
							return typecheckCppLit(state, -lemni::AInt::from(val.value));
						}
						else if constexpr(lemni::is_natural_v<Val> || std::is_same_v<Val, NaturalAInt>){
							if constexpr(std::is_same_v<Val, LemniNat16>){
								return typecheckCppLit(state, -std::int32_t(val));
							}
							else if constexpr(std::is_same_v<Val, LemniNat32>){
								return typecheckCppLit(state, -std::int64_t(val));
							}
							else if constexpr(std::is_same_v<Val, LemniNat64>){
								return typecheckCppLit(state, -lemni::AInt(val));
							}
							else{
								return typecheckCppLit(state, -val);
							}
						}
						else if constexpr(lemni::is_integer_v<Val>){
							return typecheckCppLit(state, Val(-val));
						}
						else if constexpr(lemni::is_ratio_v<Val>){
							return typecheckCppLit(state, Val{ decltype(Val::num)(-val.num), val.den });
						}
						else{
							return typecheckCppLit(state, -val);
						}
					}
					default:{ return makeError(state, LemniLocation{ UINT32_MAX, UINT32_MAX }, "undefined unary operator for numeric type"); }
				}
			}
		};

		return typedNumApply(applicator, num, state, op);
	}

	/*
	inline LemniTypecheckResult typecheckNumBinop(LemniTypecheckState state, const LemniBinaryOp op, LemniTypedNumExpr lhs, LemniTypedConstantExpr rhs){
		if(auto rhsNum = dynamic_cast<LemniTypedNumExpr>(rhs)){
			return typecheckNumBinopDispatch(state, op, lhs, rhsNum);
		}

		auto resultType = lemniBinaryOpResultType(state->types, lhs->type(), rhs->type(), op);
		auto expr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resultType, op, lhs, rhs);
		return makeResult(expr);
	}

	inline LemniTypecheckResult typecheckConstBinop(LemniTypecheckState state, const LemniBinaryOp op, LemniTypedConstantExpr lhs, LemniTypedConstantExpr rhs){
		auto lhsNum = dynamic_cast<LemniTypedNumExpr>(lhs);
		if(lhsNum) return typecheckNumBinop(state, op, lhsNum, rhs);

		auto resultType = lemniBinaryOpResultType(state->types, lhs->type(), rhs->type(), op);
		auto expr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resultType, op, lhs, rhs);
		return makeResult(expr);
	}

	inline LemniTypecheckResult typecheckBinop(LemniTypecheckState state, const LemniBinaryOp op, LemniTypedExpr lhs, LemniTypedExpr rhs){
		auto lhsConst = dynamic_cast<LemniTypedConstantExpr>(lhs);
		auto rhsConst = dynamic_cast<LemniTypedConstantExpr>(rhs);
		if(lhsConst && rhsConst)
			return typecheckConstBinop(state, op, lhsConst, rhsConst);

		auto resultType = lemniBinaryOpResultType(state->types, lhs->type(), rhs->type(), op);
		auto expr = createTypedExpr<LemniTypedBinaryOpExprT>(state, resultType, op, lhs, rhs);
		return makeResult(expr);
	}
	*/

	inline LemniTypecheckResult typecheckNumUnaryOp(LemniTypecheckState state, const LemniUnaryOp op, LemniTypedNumExpr num){
		if(auto numNat = dynamic_cast<LemniTypedNatExpr>(num)){}
		else if(auto numInt = dynamic_cast<LemniTypedIntExpr>(num)){}
		else if(auto numRat = dynamic_cast<LemniTypedRatioExpr>(num)){}
		else if(auto numReal = dynamic_cast<LemniTypedRealExpr>(num)){}

		auto resultType = lemniUnaryOpResultType(state->types, num->type(), op);
		auto expr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resultType, op, num);
		return makeResult(expr);
	}

	// TODO: pass location in to this function
	inline LemniTypecheckResult typecheckUnaryOp(LemniTypecheckState state, const LemniUnaryOp op, LemniTypedExpr val){
		if(auto valConst = dynamic_cast<LemniTypedConstantExpr>(val)){
			if(auto valNum = dynamic_cast<LemniTypedNumExpr>(valConst)){
				return typecheckNumUnaryOp(state, op, valNum);
			}
		}

		// TODO: write better error message

		auto resultType = lemniUnaryOpResultType(state->types, val->type(), op);
		if(!resultType) return litError(LemniLocation{ UINT32_MAX, UINT32_MAX }, LEMNICSTR("invalid unary op on value type"));

		auto expr = createTypedExpr<LemniTypedUnaryOpExprT>(state, resultType, op, val);

		return makeResult(expr);
	}
}

LemniTypecheckResult LemniTypedExprT::partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept{
	if(auto thisBound = bindings->find(this)) return thisBound->partialEval(state, bindings, numArgs, args);
	else if(numArgs > 0) return litError(LemniLocation{ UINT32_MAX, UINT32_MAX }, LEMNICSTR("arguments passed to non-function expression"));
	else return makeResult(this);
}

LemniTypecheckResult LemniTypedUnaryOpExprT::partialEval(
	LemniTypecheckState state, LemniPartialBindings bindings,
	const LemniNat64 numArgs, LemniTypedExpr *const args
) const noexcept{
	if(auto found = bindings->find(value)){
		auto opRes = typecheckUnaryOp(state, op, found);
		if(opRes.hasError) return opRes;
		else if(numArgs > 0) return opRes.expr->partialEval(state, bindings, numArgs, args);
		else return opRes;
	}
	else if(numArgs > 0) return litError(LemniLocation{ UINT32_MAX, UINT32_MAX }, LEMNICSTR("arguments passed to non-function expression"));
	else return makeResult(this);
}

LemniTypecheckResult LemniTypedFnDefExprT::partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept{
	LemniInt64 newArity = params.size() - numArgs;

	if(newArity < 0) return litError(LemniLocation{ UINT32_MAX, UINT32_MAX }, LEMNICSTR("too many arguments passed in function call"));

	auto fnBound = LemniPartialBindingsT(bindings);

	for(LemniNat64 i = 0; i < numArgs; i++){
		fnBound.bound[params[i]] = args[i];
	}

	return body->partialEval(state, &fnBound, 0, nullptr);
}

LemniTypedModuleExpr lemniCreateTypedModule(LemniTypecheckState state, const LemniStr alias, LemniModule module){
	auto moduleType = lemniTypeSetGetModule(state->types);
	auto moduleExpr = createTypedExpr<LemniTypedModuleExprT>(state, moduleType, module);

	auto name = alias;

	if((alias.len == 0) || !alias.ptr) name = lemniModuleId(module);

	auto aliasExpr = createTypedExpr<LemniTypedBindingExprT>(state, lemni::toStdStr(name), moduleExpr);
	lemniScopeSet(state->globalScope, aliasExpr);

	return moduleExpr;
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

		auto modRes = lemniLoadModule(state->mods, lemni::fromStdStrView(strExpr->str()));
		switch(modRes.resType){
			case LEMNI_MODULE_LEX_ERROR:{
				return makeError(
					state, loc,
					fmt::format(
						"lexing error importing module[{}.{}]: {}",
						modRes.lexErr.loc.line, modRes.lexErr.loc.col,
						lemni::toStdStrView(modRes.lexErr.msg)
					)
				);
			}
			case LEMNI_MODULE_PARSE_ERROR:{
				return makeError(
					state, loc,
					fmt::format(
						"parsing error importing module[{}.{}]: {}",
						modRes.parseErr.loc.line, modRes.parseErr.loc.col,
						lemni::toStdStrView(modRes.parseErr.msg)
					)
				);
			}
			case LEMNI_MODULE_TYPECHECK_ERROR:{
				return makeError(
					state, loc,
					fmt::format(
						"typechecking error importing module[{}.{}]: {}",
						modRes.typeErr.loc.line, modRes.typeErr.loc.col,
						lemni::toStdStrView(modRes.typeErr.msg)
					)
				);
			}
			default: break;
		}

		auto moduleType = lemniTypeSetGetModule(state->types);

		auto importExpr = createTypedExpr<LemniTypedModuleExprT>(state, moduleType, modRes.module);

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
					i + 1, lemni::toStdStrView(argType->str()), lemni::toStdStrView(paramType->str())
				);

				return makeError(state, loc, std::move(errStr));
			}

			argExprs.emplace_back(argRes.expr);
		}

		auto appExpr = createTypedExpr<LemniTypedApplicationExprT>(state, fnType->result(), fnRes.expr, std::move(argExprs));
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

LemniTypecheckResult LemniAccessExprT::typecheck(LemniTypecheckState state, LemniScope scope) const noexcept{
	auto valueRes = value->typecheck(state, scope);
	if(valueRes.hasError) return valueRes;

	LemniModule module = nullptr;

	auto expr = valueRes.expr;

	auto ref = dynamic_cast<const LemniTypedRefExprT*>(expr);
	if(ref) expr = ref->refed;

	auto bindingRef = dynamic_cast<LemniTypedBindingExpr>(expr);
	if(!bindingRef){
		return makeError(state, loc, "only module member access currently implemented");
	}

	if(auto valueMod = dynamic_cast<LemniTypedModuleExpr>(bindingRef->value)){
		module = valueMod->module;
	}
	else{
		return makeError(state, loc, "only module member access currently implemented");
	}

	if(auto rhsRef = dynamic_cast<LemniRefExpr>(access)){
		auto modState = lemniModuleTypecheckState(module);
		auto modScope = lemniTypecheckStateScope(modState);
		auto resolved = lemniScopeFind(modScope, lemni::fromStdStrView(rhsRef->id));
		if(!resolved)
			return makeError(
				state, loc,
				fmt::format(
					"could not resolve '{}' in module '{}'",
					rhsRef->id, lemni::toStdStrView(lemniModuleId(module))
				)
			);

		return makeResult(createTypedExpr<LemniTypedRefExprT>(state, resolved));
	}
	else{
		return makeError(state, loc, "only member access by static identifier currently implemented");
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
