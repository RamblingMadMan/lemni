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

#include <map>

#include "lemni/compile.h"

#include "TypedExpr.hpp"

#include "LLVM.hpp"

struct LemniCompileStateT{
	explicit LemniCompileStateT(LemniCompileState parent_ = nullptr)
		: parent(parent_), memState{lemniCreateMemCheckState(nullptr)}, llvmState("moduleId"){}

	~LemniCompileStateT(){
		lemniDestroyMemCheckState(memState);
	}

	LemniCompileState parent;
	LemniMemCheckState memState;
	lemni::LLVMState llvmState;
	std::list<std::string> errStrs;
};

struct LemniObjectT{
	//gcc_jit_result *res;
};

LemniCompileState lemniCreateCompileState(LemniCompileState parent){
	auto mem = std::malloc(sizeof(LemniCompileStateT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniCompileStateT(parent);

	return p;
}

void lemniDestroyCompileState(LemniCompileState state){
	std::destroy_at(state);
	std::free(state);
}

namespace {
	LemniObject createObject(){
		auto mem = std::malloc(sizeof(LemniObjectT));
		if(!mem) return nullptr;

		auto ptr = new(mem) LemniObjectT;

		//ptr->res = result;

		return ptr;
	}
}

LemniCompileResult lemniCompileChecked(LemniCompileState state, const LemniMemCheckExpr *expr){
	(void)state;
	(void)expr;

	LemniCompileResult res;

	res.hasError = true;
	res.error = { .msg = LEMNICSTR("compilation unimplemented"), .loc = LemniLocation{ UINT32_MAX, UINT32_MAX } };

	return res;
}

LemniCompileResult lemniCompile(LemniCompileState state, LemniTypedExpr *const exprs, const LemniNat64 numExprs){
	LemniCompileResult res;

	std::vector<llvm::Value*> rvalues;
	rvalues.reserve(numExprs);

	for(std::size_t i = 0; i < numExprs; i++){
		auto expr = exprs[i];
		auto jitRes = expr->compile(state);
		if(jitRes.hasError){
			res.hasError = true;
			res.error.msg = jitRes.error.msg;
			return res;
		}

		rvalues.emplace_back(jitRes.rvalue);
	}

	res.hasError = false;
	//res.object = createObject(result);

	return res;
}

void lemniDestroyObject(LemniObject obj){
	//gcc_jit_result_release(obj->res);

	std::destroy_at(obj);
	std::free(obj);
}

LemniFn lemniObjectFunction(LemniObject obj, const LemniStr mangledName){
	auto funcName = lemni::toStdStr(mangledName);
	//auto ret = gcc_jit_result_get_code(obj->res, funcName.c_str());
	return nullptr;
}

template<typename F, typename G>
auto compose(F f, G g){
	return [f, g](auto &&... args){ return f(g(std::forward<decltype(args)>(args)...)); };
}

using ConvFn = std::function<llvm::Value*(LemniLLVMState state, LemniType from, llvm::Value *value)>;

static ConvFn llvmConv(LemniType to){
	static auto identity = [](LemniLLVMState, LemniType, llvm::Value *value) -> llvm::Value*{ return value; };

	if(auto nat = lemniTypeAsNat(to)){
		return [nat, bits{lemniTypeNumBits(nat)}](LemniLLVMState state, LemniType, llvm::Value *value) -> llvm::Value*{
			auto type = lemniLLVMType(state, nat);
			return state->builder->CreateZExt(value, type);
		};
	}
	else if(auto int_ = lemniTypeAsInt(to)){
		return [int_, bits{lemniTypeNumBits(int_)}](LemniLLVMState state, LemniType, llvm::Value *value) -> llvm::Value*{
			auto type = lemniLLVMType(state, int_);
			return state->builder->CreateSExt(value, type);
		};
	}
	else if(auto ratio = lemniTypeAsRatio(to)){
		return [ratio, bits{lemniTypeNumBits(ratio)}](LemniLLVMState state, LemniType from, llvm::Value *value) -> llvm::Value*{
			auto ratioNum = ratio->numerator();
			auto ratioDen = ratio->denominator();

			auto newType = lemniLLVMType(state, ratio);
			auto alloca = state->builder->CreateAlloca(newType);

			auto ratioNumTy = state->builder->getIntNTy(lemniTypeNumBits(ratioNum));
			auto ratioDenTy = state->builder->getIntNTy(lemniTypeNumBits(ratioDen));

			auto ratioNumPtr = state->builder->CreateConstInBoundsGEP1_64(newType, alloca, 0);
			auto ratioDenPtr = state->builder->CreateConstInBoundsGEP1_64(newType, alloca, 1);

			auto fromType = lemniLLVMType(state, from);

			if(auto fromNat = lemniTypeAsNat(from)){
				auto natSExt = state->builder->CreateSExt(value, ratioNumTy);
				auto one = llvm::ConstantInt::get(ratioDenTy, 1, false);

				state->builder->CreateStore(natSExt, ratioNumPtr);
				state->builder->CreateStore(one, ratioDenPtr);
			}
			else if(auto fromInt = lemniTypeAsInt(from)){
				llvm::Value *intVal = value;
				if(fromInt != ratioNum){
					intVal = state->builder->CreateSExt(value, ratioNumTy);
				}

				auto one = llvm::ConstantInt::get(ratioDenTy, 1, false);

				state->builder->CreateStore(intVal, ratioNumPtr);
				state->builder->CreateStore(one, ratioDenPtr);
			}
			else if(auto fromRatio = lemniTypeAsRatio(from)){
				auto fromNumPtr = state->builder->CreateConstInBoundsGEP1_64(fromType, value, 0);
				auto fromDenPtr = state->builder->CreateConstInBoundsGEP1_64(fromType, value, 1);

				auto fromNumLoad = state->builder->CreateLoad(fromNumPtr);
				auto fromDenLoad = state->builder->CreateLoad(fromDenPtr);

				auto fromNumCast = state->builder->CreateSExt(fromNumLoad, ratioNumTy);
				auto fromDenCast = state->builder->CreateZExt(fromDenLoad, ratioDenTy);

				state->builder->CreateStore(fromNumCast, ratioNumPtr);
				state->builder->CreateStore(fromDenCast, ratioDenPtr);
			}
			else{
				return nullptr;
			}

			return state->builder->CreateLoad(newType, alloca);
		};
	}
	else if(auto real = lemniTypeAsReal(to)){
		return [real, bits{lemniTypeNumBits(real)}](LemniLLVMState state, LemniType from, llvm::Value *value) -> llvm::Value*{
			auto destTy = lemniLLVMType(state, real);

			if(auto fromNat = lemniTypeAsNat(from)){
				return state->builder->CreateUIToFP(value, destTy);
			}
			else if(auto fromInt = lemniTypeAsInt(from)){
				return state->builder->CreateSIToFP(value, destTy);
			}
			else if(auto fromRatio = lemniTypeAsRatio(from)){
				auto fromType = lemniLLVMType(state, from);

				auto fromNumPtr = state->builder->CreateConstInBoundsGEP1_64(fromType, value, 0);
				auto fromDenPtr = state->builder->CreateConstInBoundsGEP1_64(fromType, value, 1);

				auto fromNumLoad = state->builder->CreateLoad(fromNumPtr);
				auto fromDenLoad = state->builder->CreateLoad(fromDenPtr);

				auto fromNumCast = state->builder->CreateSIToFP(fromNumLoad, destTy);
				auto fromDenCast = state->builder->CreateUIToFP(fromDenLoad, destTy);

				return state->builder->CreateFDiv(fromNumCast, fromDenCast);
			}
			else if(auto fromReal = lemniTypeAsReal(from)){
				return state->builder->CreateFPExt(value, destTy);
			}
			else{
				return nullptr;
			}
		};
	}
	else{
		return identity;
	}
}

namespace {
	inline LemniJitResult litError(LemniStr msg) noexcept{
		LemniJitResult ret;
		ret.hasError = true;
		ret.error.msg = msg;
		return ret;
	}

	inline LemniJitResult strError(LemniCompileState state, std::string msg) noexcept{
		LemniJitResult ret;
		ret.hasError = true;
		ret.error.msg = lemni::fromStdStrView(state->errStrs.emplace_back(std::move(msg)));
		return ret;
	}

	using OpDispatcher = std::function<llvm::Value*(LemniLLVMState, llvm::Value*, llvm::Value*)>;

	inline OpDispatcher opDispatcher(const LemniBinaryOp op, LemniType resultType){
		switch(op){
			case LEMNI_BINARY_ADD:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFAdd(lhs, rhs);
					};
				}
				else{
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateAdd(lhs, rhs);
					};
				}
			}

			case LEMNI_BINARY_SUB:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFSub(lhs, rhs);
					};
				}
				else{
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateSub(lhs, rhs);
					};
				}
			}

			case LEMNI_BINARY_MUL:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFMul(lhs, rhs);
					};
				}
				else{
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateMul(lhs, rhs);
					};
				}
			}

			case LEMNI_BINARY_DIV:{
				return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
					return state->builder->CreateFDiv(lhs, rhs);
				};
			}

			case LEMNI_BINARY_MOD:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFRem(lhs, rhs);
					};
				}
				else if(lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateURem(lhs, rhs);
					};
				}
				else{
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateSRem(lhs, rhs);
					};
				}
			}

			case LEMNI_BINARY_POW:{
				return nullptr;
			}

			case LEMNI_BINARY_LT:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpOLT(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpSLT(lhs, rhs);
					};
				}
				else if(lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpULT(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_GT:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpOGT(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpSGT(lhs, rhs);
					};
				}
				else if(lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpUGT(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_LTEQ:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpOLE(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpSLE(lhs, rhs);
					};
				}
				else if(lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpULE(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_GTEQ:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpOGE(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpSGE(lhs, rhs);
					};
				}
				else if(lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpUGE(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_EQ:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpOEQ(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType) || lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpEQ(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_NEQ:{
				if(lemniTypeAsReal(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateFCmpONE(lhs, rhs);
					};
				}
				else if(lemniTypeAsInt(resultType) || lemniTypeAsNat(resultType)){
					return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
						return state->builder->CreateICmpNE(lhs, rhs);
					};
				}

				return nullptr;
			}

			case LEMNI_BINARY_AND:{
				return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
					return state->builder->CreateAnd(lhs, rhs);
				};
			}

			case LEMNI_BINARY_OR:{
				return [](LemniLLVMState state, auto lhs, auto rhs) -> llvm::Value*{
					return state->builder->CreateOr(lhs, rhs);
				};
			}

			case LEMNI_BINARY_CONCAT:
			default:{
				return nullptr;
			}
		}
	}
}

LemniJitResult LemniTypedBinaryOpExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	if(!lemniTypeIsCastable(lhs->type(), resultType)){
		return strError(
			state,
			fmt::format(
				"lhs of type '{}' not usable as '{}'",
				lemni::toStdStrView(lhs->type()->str()), lemni::toStdStrView(resultType->str())
			)
		);
	}
	else if(!lemniTypeIsCastable(rhs->type(), resultType)){
		return strError(
			state,
			fmt::format(
				"rhs of type '{}' not usable as '{}'",
				lemni::toStdStrView(rhs->type()->str()), lemni::toStdStrView(resultType->str())
			)
		);
	}

	auto lhsRes = lhs->compile(state, ctx);
	if(lhsRes.hasError) return lhsRes;

	auto rhsRes = rhs->compile(state, ctx);
	if(rhsRes.hasError) return rhsRes;

	auto lhsVal = lhsRes.rvalue;
	auto rhsVal = rhsRes.rvalue;

	auto conv = llvmConv(resultType);

	if(lhs->type() != resultType){
		lhsVal = conv(&state->llvmState, lhs->type(), lhsVal);
	}

	if(rhs->type() != resultType){
		rhsVal = conv(&state->llvmState, rhs->type(), rhsVal);
	}

	LemniJitResult ret;

	ret.hasError = false;

	auto dispatcher = opDispatcher(op, resultType);

	auto res = dispatcher(&state->llvmState, lhsVal, rhsVal);
	if(!res){
		ret.hasError = true;
		ret.error.msg = LEMNICSTR("operator currently unimplemented");
		return ret;
	}

	ret.rvalue = res;
	return ret;
}

LemniJitResult LemniTypedBindingExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto rvalueRes = this->value->compile(state, ctx);
	if(rvalueRes.hasError) return rvalueRes;

	auto rvalue = rvalueRes.rvalue;

	auto ty = lemniLLVMType(&state->llvmState, this->type());

	auto name = std::string(this->id());

	llvm::Value *lvalue = nullptr;

	if(ctx){
		auto allocInst = ctx->builder->CreateAlloca(ty);
		ctx->builder->CreateStore(rvalue, allocInst);
		lvalue = allocInst;
	}
	else{
		//state->llvmState.module;
		//lvalue = gcc_jit_context_new_global(state->gccState.ctx, nullptr, GCC_JIT_GLOBAL_INTERNAL, ty, name.c_str());
		// add to some global init block
	}

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = lvalue;
	return res;
}

LemniJitResult LemniTypedApplicationExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto fnRes = this->fn->compile(state, ctx);
	if(fnRes.hasError) return fnRes;

	std::vector<llvm::Value*> argValues;
	argValues.reserve(this->args.size());

	for(std::size_t i = 0; i < args.size(); i++){
		auto argRes = args[i]->compile(state, ctx);
		if(argRes.hasError) return argRes;

		argValues.emplace_back(argRes.rvalue);
	}

	auto fnRvalue = fnRes.rvalue;

	auto callValue = state->llvmState.builder->CreateCall(fnRvalue, argValues);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = callValue;
	return res;
}

LemniJitResult LemniTypedUnitExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto unitT = llvm::StructType::get(state->llvmState.ctx, "UnitT");

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantPointerNull::get(unitT->getPointerTo());
	return res;
}

LemniJitResult LemniTypedBoolExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = value ? llvm::ConstantInt::getTrue(state->llvmState.ctx) : llvm::ConstantInt::getFalse(state->llvmState.ctx);
	return res;
}

LemniJitResult LemniTypedNatNExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto natTy = llvm::Type::getIntNTy(state->llvmState.ctx, numBits);

	auto apInt = llvm::APInt(numBits, bits[0], false);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(natTy, apInt);
	return res;
}

LemniJitResult LemniTypedNat16ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto natTy = llvm::Type::getIntNTy(state->llvmState.ctx, 16);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(natTy, value);
	return res;
}

LemniJitResult LemniTypedNat32ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto natTy = llvm::Type::getIntNTy(state->llvmState.ctx, 32);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(natTy, value);
	return res;
}

LemniJitResult LemniTypedNat64ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto natTy = llvm::Type::getIntNTy(state->llvmState.ctx, 64);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(natTy, value);
	return res;
}

LemniJitResult LemniTypedIntNExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto intTy = llvm::Type::getIntNTy(state->llvmState.ctx, numBits);

	auto apInt = llvm::APInt(numBits, bits[0], true);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(intTy, apInt);
	return res;
}

LemniJitResult LemniTypedInt16ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto intTy = llvm::Type::getIntNTy(state->llvmState.ctx, 16);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(intTy, *reinterpret_cast<const LemniNat16*>(&value), true);
	return res;
}

LemniJitResult LemniTypedInt32ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto intTy = llvm::Type::getIntNTy(state->llvmState.ctx, 32);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(intTy, *reinterpret_cast<const LemniNat32*>(&value), true);
	return res;
}

LemniJitResult LemniTypedInt64ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto intTy = llvm::Type::getIntNTy(state->llvmState.ctx, 64);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantInt::get(intTy, *reinterpret_cast<const LemniNat64*>(&value), true);
	return res;
}

LemniJitResult LemniTypedReal32ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto realTy = llvm::Type::getFloatTy(state->llvmState.ctx);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantFP::get(realTy, value);
	return res;
}

LemniJitResult LemniTypedReal64ExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto realTy = llvm::Type::getDoubleTy(state->llvmState.ctx);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = llvm::ConstantFP::get(realTy, value);
	return res;
}

LemniJitResult LemniTypedProductExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	(void)ctx;

	auto prodTy = lemniLLVMType(&state->llvmState, this->productType);
	auto structTy = llvm::dyn_cast<llvm::StructType>(prodTy);

	if(isConstant){
		std::vector<llvm::Constant*> constants;
		constants.reserve(elems.size());

		for(std::size_t i = 0; i < elems.size(); i++){
			auto elem = elems[i];

			auto res = elem->compile(state);
			if(res.hasError) return res;

			auto constant = llvm::dyn_cast<llvm::Constant>(res.rvalue);

			constants.emplace_back(constant);
		}

		LemniJitResult res;
		res.hasError = false;
		res.rvalue = llvm::ConstantStruct::get(structTy, constants);
		return res;
	}
	else{
		auto alloca = state->llvmState.builder->CreateAlloca(prodTy);

		for(std::size_t i = 0; i < elems.size(); i++){
			auto elem = elems[i];

			auto res = elem->compile(state);
			if(res.hasError) return res;

			auto elemPtr = state->llvmState.builder->CreateConstInBoundsGEP1_64(prodTy, alloca, i);

			state->llvmState.builder->CreateStore(res.rvalue, elemPtr);
		}

		LemniJitResult res;
		res.hasError = false;
		res.rvalue = state->llvmState.builder->CreateLoad(prodTy, alloca);
		return res;
	}
}

LemniJitResult LemniTypedLambdaExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto llvmTy = lemniLLVMType(&state->llvmState, fnType);
	auto fnTy = llvm::dyn_cast<llvm::FunctionType>(llvmTy);

	auto lambdaFn = llvm::Function::Create(fnTy, llvm::Function::LinkageTypes::PrivateLinkage);

	LemniNat64 paramIdx = 0;
	for(auto &&arg : lambdaFn->args()){
		arg.setName(std::string(params[paramIdx]->id()));
		++paramIdx;
	}

	auto lambdaBlock = llvm::BasicBlock::Create(state->llvmState.ctx, "lbody", lambdaFn);

	auto lambdaBuilder = llvm::IRBuilder<>(lambdaBlock);
	LemniCompileContextT lambdaCtx = *ctx;
	lambdaCtx.builder = &lambdaBuilder;
	lambdaCtx.fn = lambdaFn;

	auto bodyRes = body->compile(state, &lambdaCtx);
	if(bodyRes.hasError) return bodyRes;

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = lambdaFn;
	return res;
}

LemniJitResult LemniTypedBranchExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto condRes = cond->compile(state, ctx);
	if(condRes.hasError) return condRes;

	auto trueBlock = llvm::BasicBlock::Create(state->llvmState.ctx, "condT", ctx->fn);
	auto trueBuilder = llvm::IRBuilder<>(trueBlock);
	LemniCompileContextT trueCtx = *ctx;
	trueCtx.builder = &trueBuilder;
	auto trueRes = true_->compile(state, &trueCtx);
	if(trueRes.hasError) return trueRes;

	auto falseBlock = llvm::BasicBlock::Create(state->llvmState.ctx, "condF", ctx->fn);
	auto falseBuilder = llvm::IRBuilder<>(falseBlock);
	LemniCompileContextT falseCtx = *ctx;
	falseCtx.builder = &falseBuilder;
	auto falseRes = false_->compile(state, &falseCtx);
	if(falseRes.hasError) return falseRes;

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = ctx->builder->CreateCondBr(condRes.rvalue, trueBlock, falseBlock);
	return res;
}

LemniJitResult LemniTypedReturnExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto valRes = value->compile(state, ctx);
	if(valRes.hasError) return valRes;

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = ctx->builder->CreateRet(valRes.rvalue);
	return res;
}
