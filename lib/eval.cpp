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
#include <map>
#include <unordered_map>

#include <dlfcn.h>

#include "fmt/printf.h"

#include "lemni/Value.h"
#include "lemni/eval.h"
#include "lemni/Scope.h"

#include "TypeList.hpp"
#include "TypedExpr.hpp"
#include "Value.hpp"

using namespace lemni::typelist;

struct LemniEvalStateT{
	std::vector<std::string> errMsgs;
	std::map<LemniTypedExpr, lemni::Value> stored;
	lemni::Scope globalScope;
	LemniEvalBindingsT globalBindings;
	LemniTypeSet types;
	void *dlHandle;
};

LemniEvalState lemniCreateEvalState(LemniTypeSet types){
	auto mem = std::malloc(sizeof(LemniEvalStateT));
	auto p = new(mem) LemniEvalStateT;

	p->types = types;

	p->dlHandle = dlopen(nullptr, RTLD_GLOBAL | RTLD_LAZY);
	if(!p->dlHandle){
		fmt::print(stderr, "Error in dlopen: {}\n", dlerror());
		std::destroy_at(p);
		std::free(mem);
		return nullptr;
	}

	return p;
}

void lemniDestroyEvalState(LemniEvalState state){
	dlclose(state->dlHandle);
	std::destroy_at(state);
	std::free(state);
}

namespace {
	LemniEvalResult makeError(LemniEvalState state, std::string msg){
		auto &&errMsg = state->errMsgs.emplace_back(std::move(msg));
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = lemni::fromStdStrView(errMsg);
		return res;
	}

	LemniEvalResult litError(LemniStr str){
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = str;
		return res;
	}

	LemniEvalResult makeResult(LemniValue val){
		LemniEvalResult res;
		res.hasError = false;
		res.value = val;
		return res;
	}
}

LemniEvalResult LemniTypedUnaryOpExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	auto res = value->eval(state, bindings);
	if(res.hasError) return res;

	auto retVal = lemniValueUnaryOp(this->op, res.value);

	lemniDestroyValue(res.value);

	return makeResult(retVal);
}

LemniEvalResult LemniTypedBinaryOpExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	auto res = lhs->eval(state, bindings);
	if(res.hasError) return res;

	auto lhsVal = lemni::Value::from(res.value);

	res = rhs->eval(state, bindings);
	if(res.hasError){
		return res;
	}

	auto rhsVal = lemni::Value::from(res.value);

	auto retVal = lemniValueBinaryOp(this->op, lhsVal.handle(), rhsVal.handle());

	return makeResult(retVal);
}

LemniEvalResult LemniTypedBindingExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	auto res = bindings->find(this);
	if(res){
		return makeResult(lemniCreateValueRef(res));
	}

	auto storedRes = state->stored.find(this);
	if(storedRes != end(state->stored)){
		auto valRef = lemniCreateValueRef(storedRes->second.handle());
		return makeResult(valRef);
	}

	auto valRes = value->eval(state, bindings);
	if(valRes.hasError) return valRes;

	auto val = lemni::Value::from(valRes.value);

	auto valRef = lemniCreateValueRef(val.handle());

	state->stored[this] = std::move(val);

	return makeResult(valRef);
}

LemniEvalResult LemniTypedApplicationExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	lemni::Value fnVal;
	{
		auto res = fn->eval(state, bindings);
		if(res.hasError) return res;
		fnVal = lemni::Value::from(res.value);
	}

	std::vector<lemni::Value> argVals;
	std::vector<LemniValue> argHandles;
	argVals.reserve(args.size());
	argHandles.reserve(args.size());

	for(auto arg : args){
		auto argRes = arg->eval(state, bindings);
		if(argRes.hasError) return argRes;

		auto handle = argHandles.emplace_back(argRes.value);
		argVals.emplace_back(lemni::Value::from(handle));
	}

	auto callRes = lemniValueCall(fnVal.handle(), argHandles.data(), argHandles.size());
	if(callRes.hasError){
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = callRes.error.msg;
		return res;
	}

	return makeResult(callRes.value);
}

LemniEvalResult LemniTypedProductExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	std::vector<lemni::Value> elemVals;
	std::vector<LemniValue> elemHandles;
	elemVals.reserve(elems.size());
	elemHandles.reserve(elems.size());

	for(auto elem : this->elems){
		auto elemRes = elem->eval(state, bindings);
		if(elemRes.hasError) return elemRes;

		auto handle = elemHandles.emplace_back(elemRes.value);
		elemVals.emplace_back(lemni::Value::from(handle));
	}

	return makeResult(lemniCreateValueProduct(elemHandles.data(), elemHandles.size()));
}

LemniEvalResult LemniTypedBranchExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	auto condRes = cond->eval(state, bindings);
	if(condRes.hasError) return condRes;

	auto condVal = lemniValueIsTrue(condRes.value);

	lemniDestroyValue(condRes.value);

	if(condVal == 0){
		// false
		return this->false_->eval(state, bindings);
	}
	else if(condVal == 1){
		// true
		return this->true_->eval(state, bindings);
	}
	else{
		// not a boolean condition
		return litError(LEMNICSTR("branch has non-boolean condition"));
	}
}

LemniEvalResult LemniTypedReturnExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	return litError(LEMNICSTR("return expressions can not be evaluated directly"));
}

LemniEvalResult LemniTypedBlockExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	LemniValue val = nullptr;

	for(auto expr : exprs){
		if(val) lemniDestroyValue(val);

		if(auto retExpr = dynamic_cast<LemniTypedReturnExpr>(expr)){
			auto exprRes = retExpr->value->eval(state, bindings);
			if(exprRes.hasError) return exprRes;

			return makeResult(exprRes.value);
		}
		else{
			auto exprRes = expr->eval(state, bindings);
			if(exprRes.hasError) return exprRes;

			val = exprRes.value;
		}
	}

	return makeResult(val);
}

LemniEvalResult LemniTypedLambdaExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	return litError(LEMNICSTR("lambda expression evaluation unimplemented"));
}

LemniEvalResult LemniTypedExportExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	return litError(LEMNICSTR("export expression evaluation unimplemented"));
}

LemniEvalResult LemniTypedUnitExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto unitVal = lemniCreateValueUnit();
	return makeResult(unitVal);
}

LemniEvalResult LemniTypedBoolExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto boolVal = lemniCreateValueBool(value);
	return makeResult(boolVal);
}

LemniEvalResult LemniTypedANatExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueANat(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedNatNExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)bindings;

	LemniValue val;

	const LemniNat64 natBits = bits[0];

	if(numBits <= 16){
		LemniNat16 n16;
		std::memcpy(&n16, &natBits, sizeof(n16));
		val = lemniCreateValueNat16(n16);
	}
	else if(numBits <= 32){
		LemniNat32 n32;
		std::memcpy(&n32, &natBits, sizeof(n32));
		val = lemniCreateValueNat32(n32);
	}
	else if(numBits <= 64){
		LemniNat32 n64;
		std::memcpy(&n64, &natBits, sizeof(n64));
		val = lemniCreateValueNat64(n64);
	}
	else{
		return makeError(state, "Naturals of bitwidth > 64 unimplemented");
	}

	return makeResult(val);
}

LemniEvalResult LemniTypedNat16ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueNat16(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedNat32ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueNat32(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedNat64ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueNat64(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedIntNExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)bindings;

	LemniValue val;

	if(numBits <= 64){
		LemniNat64 bitmask = 0;

		for(LemniNat64 i = 0; i < numBits; i++){
			bitmask |= 1 << i;
		}

		LemniNat64 intBits = bits[0] & bitmask;

		// https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend

		const LemniNat64 signmask = 1UL << (numBits - 1);

		// use if not already 1-filled
		intBits = intBits & ((1UL << numBits) - 1);

		const LemniNat64 extendedBits = (intBits ^ signmask) - signmask;

		if(numBits <= 16){
			LemniInt16 z16;
			std::memcpy(&z16, &extendedBits, sizeof(z16));
			val = lemniCreateValueInt16(z16);
		}
		else if(numBits <= 32){
			LemniInt32 z32;
			std::memcpy(&z32, &extendedBits, sizeof(z32));
			val = lemniCreateValueInt32(z32);
		}
		else{
			LemniInt64 z64;
			std::memcpy(&z64, &extendedBits, sizeof(z64));
			val = lemniCreateValueInt64(z64);
		}
	}
	else{
		return makeError(state, "Integers of bitwidth > 64 unimplemented");
	}

	return makeResult(val);
}

LemniEvalResult LemniTypedAIntExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueAInt(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedInt16ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueInt16(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedInt32ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueInt32(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedInt64ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueInt64(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedARatioExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueARatio(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedRatio32ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueRatio32(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedRatio64ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueRatio64(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedRatio128ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueRatio128(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedARealExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueAReal(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedReal32ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueReal32(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedReal64ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueReal64(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedStringASCIIExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueStrASCII(lemni::fromStdStrView(this->value));
	return makeResult(val);
}

LemniEvalResult LemniTypedStringUTF8ExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueStrUTF8(lemni::fromStdStrView(this->value));
	return makeResult(val);
}

LemniEvalResult LemniTypedModuleExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)bindings;

	auto storedRes = state->stored.find(this);
	if(storedRes != end(state->stored)){
		auto ref = lemniCreateValueRef(storedRes->second.handle());
		return makeResult(ref);
	}

	auto val = lemniCreateValueModule(this->module);

	auto ref = lemniCreateValueRef(val);

	state->stored[this] = lemni::Value::from(val);

	return makeResult(ref);
}

LemniEvalResult LemniTypedTypeExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;
	(void)bindings;
	auto val = lemniCreateValueType(this->value);
	return makeResult(val);
}

LemniEvalResult LemniTypedParamBindingExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	(void)state;

	LemniEvalResult ret;

	auto res = bindings->find(this);
	if(!res){
		return litError(LEMNICSTR("no value is bound to parameter"));
	}
	else{
		return makeResult(lemniCreateValueRef(res));
	}
}

LemniEvalResult LemniTypedFnDefExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	auto doType = [](void *const self_, LemniTypeSet types) -> LemniType{
		auto self = reinterpret_cast<LemniTypedFnDefExpr>(self_);
		auto fnType = self->lambda->fnType;
		// TODO: ensure 'fnType' exists in 'types'
		(void)types;
		return fnType;
	};

	auto doEval = [](void *const self_, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs){
		auto self = reinterpret_cast<LemniTypedFnDefExpr>(self_);

		auto fnType = self->lambda->fnType;

		if(numArgs != self->lambda->params.size()){
			return litError(LEMNICSTR("wrong number of args passed"));
		}

		auto fnBindings = LemniEvalBindingsT(bindings);

		for(std::size_t i = 0; i < self->lambda->params.size(); i++){
			fnBindings.bound[self->lambda->params[i]] = lemni::Value::from(lemniCreateValueRef(args[i]));
		}

		auto retRes = self->lambda->body->eval(state, &fnBindings);
		return retRes;
	};

	auto val = lemniCreateValueFn(doType, doEval, const_cast<LemniTypedFnDefExprT*>(this), state, bindings);
	return makeResult(val);
}

template<typename...>
struct LemniTypeApp;

template<bool ConvertC, template<typename...> typename F, typename ... Ts>
struct LemniTypeApp<std::integral_constant<bool, ConvertC>, Partial<F, Ts...>>{
	template<typename ... Us>
	using Call = F<Ts..., Us...>;

	template<typename ... Args>
	static decltype(auto) apply(LemniType type, Args &&... args){
		if(lemniTypeAsBottom(type)){
			return Call<void>::apply(std::forward<Args>(args)...);
		}
		else if(lemniTypeAsUnit(type)){
			return Call<std::conditional_t<ConvertC, LemniUnit, void>>::apply(std::forward<Args>(args)...);
		}
		else if(auto boolTy = lemniTypeAsBool(type)){
			return Call<bool>::apply(std::forward<Args>(args)...);
		}
		else if(auto natTy = lemniTypeAsNat(type)){
			switch(lemniTypeNumBits(type)){
				case 16: return Call<std::uint16_t>::apply(std::forward<Args>(args)...);
				case 32: return Call<std::uint32_t>::apply(std::forward<Args>(args)...);
				case 64: return Call<std::uint64_t>::apply(std::forward<Args>(args)...);
				default:{
					assert(!"natural type of non-system bits");
					break;
				}
			}
		}
		else if(auto intTy = lemniTypeAsInt(type)){
			switch(lemniTypeNumBits(type)){
				case 16: return Call<std::int16_t>::apply(std::forward<Args>(args)...);
				case 32: return Call<std::int32_t>::apply(std::forward<Args>(args)...);
				case 64: return Call<std::int64_t>::apply(std::forward<Args>(args)...);
				default:{
					assert(!"integer type of non-system bits");
					break;
				}
			}
		}
		else if(auto ratioTy = lemniTypeAsRatio(type)){
			switch(lemniTypeNumBits(type)){
				case 32: return Call<LemniRatio32>::apply(std::forward<Args>(args)...);
				case 64: return Call<LemniRatio64>::apply(std::forward<Args>(args)...);
				case 128: return Call<LemniRatio128>::apply(std::forward<Args>(args)...);
				default:{
					assert(!"ratio type of non-system bits");
					break;
				}
			}
		}
		else if(auto realTy = lemniTypeAsReal(type)){
			switch(lemniTypeNumBits(type)){
				case 32: return Call<float>::apply(std::forward<Args>(args)...);
				case 64: return Call<double>::apply(std::forward<Args>(args)...);
				default:{
					assert(!"real type of non-system bits");
					break;
				}
			}
		}
		else if(auto strTy = lemniTypeAsString(type)){
			return Call<LemniStr>::apply(std::forward<Args>(args)...);
		}
		else{
			assert(!"un-representable type");
		}
	}
};

template<typename...>
struct TypedFnCaller;

template<typename Result>
struct TypedFnCaller<Result()>{
	static LemniEvalFn apply() noexcept{
		return [](void *const ptr, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs){
			(void)bindings;

			auto fptr = reinterpret_cast<Result(*)()>(ptr);

			if(numArgs != 0){
				if(numArgs == 1){
					auto argType = args[0]->getType(state->types);
					if(!lemniTypeAsUnit(argType)){
						return litError(LEMNICSTR("wrong number of args passed"));
					}
				}
				else{
					return litError(LEMNICSTR("wrong number of args passed"));
				}
			}

			LemniValueCallResult res;
			res.hasError = false;

			if constexpr(std::is_same_v<void, Result>){
				fptr();
				res.value = lemniCreateValueUnit();
			}
			else{
				res.value = lemni::detail::createLemniValue(fptr());
			}

			return res;
		};
	}
};

template<typename Result, typename ... Params>
struct TypedFnCaller<Result(Params...)>{
	template<std::size_t ... ParamsIndices>
	static LemniEvalFn applyImpl(std::index_sequence<ParamsIndices...>){
		return [](void *const ptr, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs){
			(void)bindings;

			auto fptr = reinterpret_cast<Result(*)(Params...)>(ptr);

			if(numArgs != sizeof...(Params)){
				return litError(LEMNICSTR("wrong number of args passed"));
			}

			std::unique_ptr<std::byte[]> argDatas[] = { argData<ParamsIndices>(state->types, args)... };

			auto argPtrs = std::make_tuple(argPtr<ParamsIndices>(argDatas)...);

			LemniValueCallResult res;
			res.hasError = false;

			if constexpr(std::is_same_v<void, Result>){
				fptr(*std::get<ParamsIndices>(argPtrs)...);
				res.value = lemniCreateValueUnit();
			}
			else{
				res.value = lemni::detail::createLemniValue(fptr(*std::get<ParamsIndices>(argPtrs)...));
			}

			return res;
		};
	}

	static LemniEvalFn apply() noexcept{
		return applyImpl(std::make_index_sequence<sizeof...(Params)>{});
	}

	template<std::size_t Idx>
	static auto argPtr(std::unique_ptr<std::byte[]> (&argDatas)[sizeof...(Params)]) noexcept{
		using ParamType = std::tuple_element_t<Idx, std::tuple<Params...>>;
		return reinterpret_cast<ParamType*>(argDatas[Idx].get());
	}

	template<std::size_t Idx>
	static auto argData(LemniTypeSet types, LemniValue *const args) noexcept{
		using ParamType = std::tuple_element_t<Idx, std::tuple<Params...>>;
		auto paramTy = lemni::getCType<ParamType>(types);
		return args[Idx]->toBytes(paramTy);
	}
};

template<typename...>
struct TypedFFICallerRetPartial;

template<std::size_t ParamsRem, typename Ret, typename ... Params>
struct TypedFFICallerRetPartial<std::integral_constant<std::size_t, ParamsRem>, Ret, Params...>{
	static auto apply(LemniType *const params){
		return LemniTypeApp<std::true_type, TypedFFICallerRetPartial, std::integral_constant<std::size_t, ParamsRem-1>, Ret, Params...>::apply(params[0], params + 1);
	}
};

template<typename Ret, typename ... Params>
struct TypedFFICallerRetPartial<std::integral_constant<std::size_t, 0>, Ret, Params...>{
	static auto apply(LemniType *const params){
		(void)params;
		return TypedFnCaller<Ret(Params...)>::apply();
	}
};

template<typename...>
struct TypedFFICallerRet;

template<std::size_t NumParams, typename Ret>
struct TypedFFICallerRet<std::integral_constant<std::size_t, NumParams>, Ret>{
	static auto apply(LemniType *const params){
		return TypedFFICallerRetPartial<std::integral_constant<std::size_t, NumParams>, Ret>::apply(params);
	}
};

template<std::size_t NumParams>
struct TypedFFICaller{
	static_assert(!(NumParams >= 0) || !(NumParams <= 0));
	static auto apply(LemniType result, LemniType *const params){
		return LemniTypeApp<std::false_type, Partial<TypedFFICallerRet, std::integral_constant<std::size_t, NumParams>>>::apply(result, params);
	}
};

template<typename...>
struct FFICallerGet;

template<std::size_t NumParams, typename Ret>
struct FFICallerGet<std::integral_constant<std::size_t, NumParams>, Ret>{
	template<typename T, std::size_t I, std::size_t ... Is, typename ... Ts>
	static auto appendTypesImpl(LemniType *const params, TypeList<Ts...>){
		if constexpr(sizeof...(Is) == 0){
			return TypedFnCaller<Ret(Ts..., T)>::apply();
		}
		else{
			return appendTypes<Is...>(params, TypeList<Ts..., T>{});
		}
	}

	template<std::size_t I, std::size_t ... Is, typename ... Ts>
	static auto appendTypes(LemniType *const params, TypeList<Ts...> tl){
		auto param = params[I];

		//auto paramTypes = std::make_tuple(paramTypeList<I>(params), paramTypeList<Is>(params)...);
		if(lemniTypeAsUnit(param)) return appendTypesImpl<LemniUnit, I, Is...>(params, tl);
		else if(lemniTypeAsBool(param)) return appendTypesImpl<LemniBool, I, Is...>(params, tl);
		else if(lemniTypeAsNat(param)){
			switch(lemniTypeNumBits(param)){
				case 16: return appendTypesImpl<LemniNat16, I, Is...>(params, tl);
				case 32: return appendTypesImpl<LemniNat32, I, Is...>(params, tl);
				case 64: return appendTypesImpl<LemniNat64, I, Is...>(params, tl);
				default: assert(!"unsupported natural bit width");
			}
		}
		else if(lemniTypeAsInt(param)){
			switch(lemniTypeNumBits(param)){
				case 16: return appendTypesImpl<LemniInt16, I, Is...>(params, tl);
				case 32: return appendTypesImpl<LemniInt32, I, Is...>(params, tl);
				case 64: return appendTypesImpl<LemniInt64, I, Is...>(params, tl);
				default: assert(!"unsupported integer bit width");
			}
		}
		else if(lemniTypeAsRatio(param)){
			switch(lemniTypeNumBits(param)){
				case 32: return appendTypesImpl<LemniRatio32, I, Is...>(params, tl);
				case 64: return appendTypesImpl<LemniRatio64, I, Is...>(params, tl);
				case 128: return appendTypesImpl<LemniRatio128, I, Is...>(params, tl);
				default: assert(!"unsupported rational bit width");
			}
		}
		else if(lemniTypeAsReal(param)){
			switch(lemniTypeNumBits(param)){
				case 32: return appendTypesImpl<LemniReal32, I, Is...>(params, tl);
				case 64: return appendTypesImpl<LemniReal64, I, Is...>(params, tl);
				default: assert(!"unsupported real bit width");
			}
		}
		else{
			assert(!"un-representable type");
		}
	}

	template<std::size_t ... Is>
	static auto applyImpl(LemniType *const params, std::index_sequence<Is...>){
		return appendTypes<Is...>(params, TypeList<>{});
	}

	static auto apply(LemniType *const params){
		return applyImpl(params, std::make_index_sequence<NumParams>{});
	}
};

template<typename Ret>
struct FFICallerGet<std::integral_constant<std::size_t, 0>, Ret>{
	static auto apply(){
		return TypedFnCaller<Ret()>::apply();
	}
};

namespace detail{
	template<auto...>
	struct MinT;

	template<auto...>
	struct MaxT;

	template<typename T, T lhs, T rhs>
	struct MinT<lhs, rhs>{
		static constexpr T value = lhs < rhs ? lhs : rhs;
	};

	template<typename T, T lhs, T rhs>
	struct MaxT<lhs, rhs>{
		static constexpr T value = lhs > rhs ? lhs : rhs;
	};

	template<auto ... Args>
	inline constexpr auto Min = MinT<Args...>::value;

	template<auto ... Args>
	inline constexpr auto Max = MaxT<Args...>::value;
}

template<typename Ret>
struct FFICallerRet{
	static LemniEvalFn apply(LemniTypedExtFnDeclExpr fn){
		static auto callFn = +[](LemniTypedExtFnDeclExpr self, LemniEvalState state, const char *name, void *argPtrs[]){
			auto fnptr = self->ptr;
			if(!fnptr){
				fnptr = dlsym(state->dlHandle, name);
				if(!fnptr)
					return makeError(state, fmt::format("Error in dlsym: {}", dlerror()));
			}

			if constexpr(std::is_same_v<void, Ret>){
				ffi_arg result; // probably not needed
				ffi_call(&self->cif, FFI_FN(fnptr), &result, argPtrs);

				LemniValueCallResult res;
				res.hasError = false;
				res.value = lemniCreateValueUnit();
				return res;
			}
			else{
				thread_local std::aligned_storage_t<
					detail::Max<sizeof(Ret), sizeof(ffi_arg)>,
					detail::Max<alignof(Ret), alignof(ffi_arg)>> retStorage;

				ffi_call(&self->cif, FFI_FN(fnptr), &retStorage, argPtrs);

				LemniValueCallResult res;
				res.hasError = false;
				res.value = lemni::detail::createLemniValue(*reinterpret_cast<Ret*>(&retStorage));
				return res;
			}
		};

		if((fn->fnType->numParams() == 1) && lemniTypeAsUnit(fn->fnType->param(0))){
			return +[](void *const ptr, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs)
					-> LemniValueCallResult
			{
				(void)bindings;
				auto self = reinterpret_cast<LemniTypedExtFnDeclExpr>(ptr);
				auto name = std::string(self->id());

				if(numArgs != 0){
					if(numArgs == 1){
						auto argType = args[0]->getType(state->types);
						if(!lemniTypeAsUnit(argType)){
							return litError(LEMNICSTR("wrong number of args passed"));
						}
					}
					else{
						return litError(LEMNICSTR("wrong number of args passed"));
					}
				}

				return callFn(self, state, name.c_str(), nullptr);
			};
		}
		else{
			return +[](void *const ptr, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs)
					-> LemniValueCallResult
			{
				(void)bindings;
				auto self = reinterpret_cast<LemniTypedExtFnDeclExpr>(ptr);
				auto fnType = self->fnType;
				auto name = std::string(self->id());

				if(numArgs != self->paramNames.size()){
					return litError(LEMNICSTR("wrong number of args passed"));
				}

				std::vector<std::unique_ptr<std::byte[]>> argBytes;
				std::vector<void*> argPtrs;

				argBytes.reserve(numArgs);
				argPtrs.reserve(numArgs);

				for(LemniNat64 i = 0; i < numArgs; i++){
					auto arg = args[i];
					auto argType = arg->getType(state->types);
					auto paramType = lemniFunctionTypeParam(fnType, i);

					auto bytes = arg->toBytes(paramType);
					if(!bytes){
						return makeError(state, fmt::format(
							"argument {} is not castable from '{}' to '{}'",
							i+1,
							lemni::toStdStrView(lemniTypeStr(argType)),
							lemni::toStdStrView(lemniTypeStr(paramType))
						));
					}

					argPtrs.emplace_back(bytes.get());
					argBytes.emplace_back(std::move(bytes));
				}

				return callFn(self, state, name.c_str(), argPtrs.data());
			};
		}
	}
};

struct FFICaller{
	static auto get(LemniType result, LemniTypedExtFnDeclExpr fn){
		return LemniTypeApp<std::false_type, Partial<FFICallerRet>>::apply(result, fn);
	}
};

LemniEvalResult LemniTypedExtFnDeclExprT::eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept{
	LemniTypeFn typeFn = [](void *const self_, LemniTypeSet types) -> LemniType{
		auto self = reinterpret_cast<LemniTypedExtFnDeclExpr>(self_);
		auto fnType = self->fnType;
		// TODO: ensure 'fnType' exists in 'types'
		(void)types;
		return fnType;
	};

	LemniEvalFn evalFn;

	auto name = std::string(id());

	void *data = const_cast<LemniTypedExtFnDeclExprT*>(this);

	auto numParams = lemniFunctionTypeNumParams(fnType);

	auto resultType = lemniFunctionTypeResult(fnType);

	std::vector<LemniType> paramTypes;
	paramTypes.reserve(numParams);

	for(std::size_t i = 0; i < numParams; i++){
		paramTypes.emplace_back(lemniFunctionTypeParam(fnType, i));
	}

#define LEMNI_FN_CASE(n)\
		case n:{\
			data = ptr;\
			evalFn = LemniTypeApp<\
				std::false_type,\
				Partial<FFICallerGet, std::integral_constant<std::size_t, n>>\
			>::apply(resultType, paramTypes.data());\
			break;\
		}

	switch(lemniFunctionTypeNumParams(fnType)){
		//LEMNI_FN_CASE(1)
		//LEMNI_FN_CASE(2)
		/*
		LEMNI_FN_CASE(3)
		LEMNI_FN_CASE(4)
		LEMNI_FN_CASE(5)
		LEMNI_FN_CASE(6)
		LEMNI_FN_CASE(7)
		LEMNI_FN_CASE(8)
		LEMNI_FN_CASE(9)
		LEMNI_FN_CASE(10)
		LEMNI_FN_CASE(11)
		LEMNI_FN_CASE(12)
		LEMNI_FN_CASE(13)
		LEMNI_FN_CASE(14)
		LEMNI_FN_CASE(15)
		LEMNI_FN_CASE(16)
		*/
		default:{ evalFn = FFICaller::get(resultType, this); break; }
	}

#undef LEMNI_FN_CASE

	auto val = lemniCreateValueFn(typeFn, evalFn, data, state, bindings);
	return makeResult(val);
}

LemniEvalBindings lemniEvalGlobalBindings(LemniEvalState state){
	return &state->globalBindings;
}

LemniEvalResult lemniEval(LemniEvalState state, LemniTypedExpr expr){
	return expr->eval(state, &state->globalBindings);
}
