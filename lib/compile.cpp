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
		: parent(parent_), llvmState("moduleId"){}

	LemniCompileState parent;
	lemni::LLVMState llvmState;
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

using ConvFn = std::function<llvm::Value*(LemniLLVMState state, llvm::Value *value)>;

static ConvFn llvmConv(LemniType to){
	static auto identity = [](LemniLLVMState, llvm::Value *value){ return value; };

	return identity;
}

LemniJitResult LemniTypedBinaryOpExprT::compile(LemniCompileState state, LemniCompileContext ctx) const noexcept{
	auto lhsRes = this->lhs->compile(state, ctx);
	if(lhsRes.hasError) return lhsRes;

	auto rhsRes = this->rhs->compile(state, ctx);
	if(rhsRes.hasError) return rhsRes;

	LemniJitResult ret;

	ret.hasError = false;

	auto retType = lemniLLVMType(&state->llvmState, resultType);

	switch(this->op){
		case LEMNI_BINARY_ADD:{
			ret.rvalue = ctx->builder->CreateAdd(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_SUB:{
			ret.rvalue = ctx->builder->CreateSub(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_MUL:{
			ret.rvalue = ctx->builder->CreateMul(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_DIV:{
			ret.rvalue = ctx->builder->CreateFDiv(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_MOD:{
			ret.rvalue = ctx->builder->CreateSRem(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_POW:{
			ret.hasError = true;
			ret.error.msg = LEMNICSTR("power operator currently unimplemented");
			return ret;
		}

		case LEMNI_BINARY_LT:{
			ret.rvalue = ctx->builder->CreateFCmpOLT(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_GT:{
			ret.rvalue = ctx->builder->CreateFCmpOGT(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_LTEQ:{
			ret.rvalue = ctx->builder->CreateFCmpOLE(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_GTEQ:{
			ret.rvalue = ctx->builder->CreateFCmpOGE(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_EQ:{
			ret.rvalue = ctx->builder->CreateFCmpOEQ(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_NEQ:{
			ret.rvalue = ctx->builder->CreateFCmpONE(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_AND:{
			ret.rvalue = ctx->builder->CreateAnd(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_OR:{
			ret.rvalue = ctx->builder->CreateOr(lhsRes.rvalue, rhsRes.rvalue);
			return ret;
		}

		case LEMNI_BINARY_CONCAT:{
			ret.hasError = true;
			ret.error.msg = LEMNICSTR("concatenation operator unimplemented");
			return ret;
		}

		default:{
			ret.hasError = true;
			ret.error.msg = LEMNICSTR("unsupported comparison operator");
			return ret;
		}
	}
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

	LemniJitResult res;
	res.hasError = true;
	res.error.msg = LEMNICSTR("function application compilation unimplemented");
	//res.rvalue = gcc_jit_context_new_call_through_ptr(state->gccState.ctx, nullptr, fnRvalue, argValues.size(), argValues.data());
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

	(void)prodTy;

	llvm::Value *rvalue = nullptr;

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

		rvalue = llvm::ConstantStruct::get(structTy, constants);
	}
	else{
		std::vector<llvm::Value*> rvalues;
		rvalues.reserve(elems.size());

		for(std::size_t i = 0; i < elems.size(); i++){
			auto elem = elems[i];

			auto res = elem->compile(state);
			if(res.hasError) return res;

			rvalues.emplace_back(res.rvalue);
		}

		//auto allocInst = llvm::IRBuilder<>::CreateAlloca(prodTy);
	}

	LemniJitResult res;
	res.hasError = true;
	res.error.msg = LEMNICSTR("product literal compilation unimplemented");
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
