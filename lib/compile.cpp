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

#include <libgccjit.h>

#include "lemni/compile.h"

#include "TypedExpr.hpp"

struct LemniJitExprT{
	gcc_jit_rvalue *handle;
};

struct LemniCompileStateT{
	LemniCompileState parent;
	gcc_jit_context *ctx;

	std::map<LemniType, gcc_jit_type*> jitTypes;
	std::map<LemniTypedFnDefExpr, gcc_jit_function*> jitFns;
};

gcc_jit_type *lemniMakeGccjitType(LemniCompileState state, const LemniType type);

gcc_jit_type *lemniFindGccjitType(LemniCompileState state, const LemniType type){
	auto tyRes = state->jitTypes.find(type);
	if(tyRes != end(state->jitTypes)) return tyRes->second;
	else if(state->parent) return lemniFindGccjitType(state->parent, type);
	else return nullptr;
}

gcc_jit_type *lemniGccjitType(LemniCompileState state, const LemniType type){
	auto ty = lemniFindGccjitType(state, type);
	if(ty) return ty;

	ty = lemniMakeGccjitType(state, type);

	state->jitTypes[type] = ty;

	return ty;
}

gcc_jit_type *lemniMakeGccjitType(LemniCompileState state, const LemniType type){
	if(lemniTypeAsUnit(type)){
		 auto unitTStruct = gcc_jit_context_new_opaque_struct(state->ctx, nullptr, "UnitT");
		 auto unitTTy = gcc_jit_type_get_const(gcc_jit_struct_as_type(unitTStruct));
		 auto unitTy = gcc_jit_type_get_const(gcc_jit_type_get_pointer(unitTTy));
		 return unitTy;
	}
	else if(lemniTypeAsNat(type)){
		auto numBits = lemniTypeNumBits(type);

		if((numBits == 0) || (numBits > 64)){
			return nullptr;
		}

		auto numBytes = numBits / 8;
		auto remBits = numBits % 8;

		if(remBits > 0){
			return nullptr;
		}

		return gcc_jit_context_get_int_type(state->ctx, numBytes, 0);
	}
	else if(lemniTypeAsInt(type)){
		auto numBits = lemniTypeNumBits(type);

		if((numBits == 0) || (numBits > 64)){
			return nullptr;
		}

		auto numBytes = numBits / 8;
		auto remBits = numBits % 8;

		if(remBits > 0){
			return nullptr;
		}

		return gcc_jit_context_get_int_type(state->ctx, numBytes, 1);
	}
	else if(lemniTypeAsRatio(type)){
		auto numBits = lemniTypeNumBits(type);

		if((numBits == 0) || (numBits > 128)){
			return nullptr;
		}

		auto numBytes = numBits / 16;
		auto remBits = numBits % 16;

		if(remBits > 0){
			return nullptr;
		}

		auto numTy = gcc_jit_context_get_int_type(state->ctx, numBytes, 1);
		auto denTy = gcc_jit_context_get_int_type(state->ctx, numBytes, 0);

		gcc_jit_type *fieldTys[2] = {numTy, denTy};
		const char *fieldNames[2] = {"num", "den"};
		gcc_jit_field *fields[2];

		for(std::size_t i = 0; i < 2; i++){
			fields[i] = gcc_jit_context_new_field(state->ctx, nullptr, fieldTys[i], fieldNames[i]);
		}

		auto typeName = "Ratio" + std::to_string(numBits);

		auto ratioStruct = gcc_jit_context_new_struct_type(state->ctx, nullptr, typeName.c_str(), 2, fields);

		return gcc_jit_struct_as_type(ratioStruct);
	}
	else if(lemniTypeAsReal(type)){
		auto numBits = lemniTypeNumBits(type);

		switch(numBits){
			case 32: return gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_FLOAT);
			case 64: return gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_DOUBLE);
			default: return nullptr;
		}
	}
	else if(lemniTypeAsStringUTF8(type)){
		auto ptrTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_CONST_CHAR_PTR);
		auto sizeTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_SIZE_T);

		gcc_jit_type *fieldTys[2] = {ptrTy, sizeTy};
		const char *fieldsNames[2] = {"ptr", "len"};
		gcc_jit_field *fields[2];

		for(std::size_t i = 0; i < 2; i++){
			fields[i] = gcc_jit_context_new_field(state->ctx, nullptr, fieldTys[i], fieldsNames[i]);
		}

		auto strStruct = gcc_jit_context_new_struct_type(state->ctx, nullptr, "StringUTF8", 2, fields);

		return gcc_jit_struct_as_type(strStruct);
	}
	else if(lemniTypeAsStringASCII(type)){
		auto ptrTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_CONST_CHAR_PTR);
		auto sizeTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_SIZE_T);

		gcc_jit_type *fieldTys[2] = {ptrTy, sizeTy};
		const char *fieldsNames[2] = {"ptr", "len"};
		gcc_jit_field *fields[2];

		for(std::size_t i = 0; i < 2; i++){
			fields[i] = gcc_jit_context_new_field(state->ctx, nullptr, fieldTys[i], fieldsNames[i]);
		}

		auto strStruct = gcc_jit_context_new_struct_type(state->ctx, nullptr, "StringASCII", 2, fields);

		return gcc_jit_struct_as_type(strStruct);
	}
	else if(auto prod = lemniTypeAsProduct(type)){
		auto numComps = lemniProductTypeNumComponents(prod);

		std::vector<gcc_jit_field*> fields;
		fields.reserve(numComps);

		for(std::uint32_t i = 0; i < numComps; i++){
			auto comp = lemniProductTypeComponent(prod, i);
			auto ty = lemniGccjitType(state, comp);
			auto compName = "_" + std::to_string(i);
			fields.emplace_back(gcc_jit_context_new_field(state->ctx, nullptr, ty, compName.c_str()));
		}

		auto prodMangled = lemni::toStdStr(lemniTypeMangled(type));
		auto prodStruct = gcc_jit_context_new_struct_type(state->ctx, nullptr, prodMangled.c_str(), numComps, fields.data());

		return gcc_jit_struct_as_type(prodStruct);
	}
	else if(auto fn = lemniTypeAsFunction(type)){
		auto retTy = lemniGccjitType(state, lemniFunctionTypeResult(fn));
		auto numParams = lemniFunctionTypeNumParams(fn);

		std::vector<gcc_jit_type*> paramTypes;
		paramTypes.reserve(numParams);

		for(std::uint32_t i = 0; i < numParams; i++){
			paramTypes.emplace_back(lemniGccjitType(state, lemniFunctionTypeParam(fn, i)));
		}

		return gcc_jit_context_new_function_ptr_type(state->ctx, nullptr, retTy, numParams, paramTypes.data(), 0);
	}
	else{
		assert(!"type not representable");
	}
}

struct LemniObjectT{
	gcc_jit_result *res;
};

LemniCompileState lemniCreateCompileState(LemniCompileState parent){
	auto mem = std::malloc(sizeof(LemniCompileStateT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniCompileStateT;

	p->parent = parent;
	if(p->parent){
		p->ctx = gcc_jit_context_new_child_context(parent->ctx);
	}
	else{
		p->ctx = gcc_jit_context_acquire();
	}

	return p;
}

void lemniDestroyCompileState(LemniCompileState state){
	gcc_jit_context_release(state->ctx);

	std::destroy_at(state);
	std::free(state);
}

namespace {
	LemniObject createObject(gcc_jit_result *result){
		auto mem = std::malloc(sizeof(LemniObjectT));
		if(!mem) return nullptr;

		auto ptr = new(mem) LemniObjectT;

		ptr->res = result;

		return ptr;
	}
}

LemniCompileResult lemniCompile(LemniCompileState state, LemniTypedExpr *const exprs, const size_t numExprs){
	LemniCompileResult res;

	auto thisState = lemniCreateCompileState(state);

	std::vector<gcc_jit_rvalue*> rvalues;
	rvalues.reserve(numExprs);

	for(std::size_t i = 0; i < numExprs; i++){
		auto expr = exprs[i];
		auto jitRes = expr->compile(thisState);
		if(jitRes.hasError){
			res.hasError = true;
			res.error.msg = jitRes.error.msg;
			return res;
		}

		rvalues.emplace_back(jitRes.rvalue);
	}

	auto result = gcc_jit_context_compile(thisState->ctx);
	if(!result){
		res.hasError = true;
		res.error.msg = LEMNICSTR("error in gcc_jit_context_compile");
		return res;
	}

	lemniDestroyCompileState(thisState);

	res.hasError = false;
	res.object = createObject(result);

	return res;
}

void lemniDestroyObject(LemniObject obj){
	gcc_jit_result_release(obj->res);

	std::destroy_at(obj);
	std::free(obj);
}

LemniFn lemniObjectFunction(LemniObject obj, const LemniStr mangledName){
	auto funcName = lemni::toStdStr(mangledName);
	auto ret = gcc_jit_result_get_code(obj->res, funcName.c_str());
	return reinterpret_cast<LemniFn>(ret);
}

/*
 *
 *  Type expression compilation implementation
 *
 */

gcc_jit_binary_op lemniGccjitBinaryOp(LemniBinaryOp op){
	switch(op){
		case LEMNI_BINARY_ADD: return GCC_JIT_BINARY_OP_PLUS;
		case LEMNI_BINARY_SUB: return GCC_JIT_BINARY_OP_MINUS;
		case LEMNI_BINARY_MUL: return GCC_JIT_BINARY_OP_MULT;
		case LEMNI_BINARY_DIV: return GCC_JIT_BINARY_OP_DIVIDE;
		case LEMNI_BINARY_MOD: return GCC_JIT_BINARY_OP_MODULO;
		case LEMNI_BINARY_AND: return GCC_JIT_BINARY_OP_LOGICAL_AND;
		case LEMNI_BINARY_OR: return GCC_JIT_BINARY_OP_LOGICAL_OR;
		default: assert(!"binary op is not an arithmetic operator");
	}
}

gcc_jit_comparison lemniGccjitComparison(LemniBinaryOp op){
	switch(op){
		case LEMNI_BINARY_EQ: return GCC_JIT_COMPARISON_EQ;
		case LEMNI_BINARY_NEQ: return GCC_JIT_COMPARISON_NE;
		case LEMNI_BINARY_LT: return GCC_JIT_COMPARISON_LT;
		case LEMNI_BINARY_GT: return GCC_JIT_COMPARISON_GT;
		case LEMNI_BINARY_LTEQ: return GCC_JIT_COMPARISON_LE;
		case LEMNI_BINARY_GTEQ: return GCC_JIT_COMPARISON_GE;
		default: assert(!"binary op is not a comparison operator");
	}
}

LemniJitResult LemniTypedBinaryOpExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	auto lhsRes = this->lhs->compile(state, block);
	if(lhsRes.hasError) return lhsRes;

	auto rhsRes = this->rhs->compile(state, block);
	if(rhsRes.hasError) return rhsRes;

	LemniJitResult ret;

	ret.hasError = false;

	if(lemniBinaryOpIsComparison(this->op)){
		auto gccOp = lemniGccjitComparison(this->op);
		ret.rvalue = gcc_jit_context_new_comparison(state->ctx, nullptr, gccOp, lhsRes.rvalue, rhsRes.rvalue);
	}
	else{
		auto resTy = lemniGccjitType(state, this->type());
		auto gccOp = lemniGccjitBinaryOp(this->op);
		ret.rvalue = gcc_jit_context_new_binary_op(state->ctx, nullptr, gccOp, resTy, lhsRes.rvalue, rhsRes.rvalue);
	}

	return ret;
}

LemniJitResult LemniTypedBindingExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	auto rvalueRes = this->value->compile(state, block);
	if(rvalueRes.hasError) return rvalueRes;

	auto rvalue = rvalueRes.rvalue;

	auto ty = lemniGccjitType(state, this->type());

	auto name = std::string(this->id());

	gcc_jit_lvalue *lvalue = nullptr;

	if(block){
		auto fn = gcc_jit_block_get_function(block);
		lvalue = gcc_jit_function_new_local(fn, nullptr, ty, name.c_str());
		gcc_jit_block_add_assignment(block, nullptr, lvalue, rvalue);
	}
	else{
		lvalue = gcc_jit_context_new_global(state->ctx, nullptr, GCC_JIT_GLOBAL_INTERNAL, ty, name.c_str());
		// add to some global init block
	}

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_lvalue_as_rvalue(lvalue);
	return res;
}

LemniJitResult LemniTypedApplicationExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	auto fnRes = this->fn->compile(state, block);
	if(fnRes.hasError) return fnRes;

	std::vector<gcc_jit_rvalue*> argValues;
	argValues.reserve(this->args.size());

	for(std::size_t i = 0; i < args.size(); i++){
		auto argRes = args[i]->compile(state, block);
		if(argRes.hasError) return argRes;

		argValues.emplace_back(argRes.rvalue);
	}

	auto fnRvalue = fnRes.rvalue;

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_call_through_ptr(state->ctx, nullptr, fnRvalue, argValues.size(), argValues.data());
	return res;
}

LemniJitResult LemniTypedUnitExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto unitTy = lemniGccjitType(state, this->unitType);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_null(state->ctx, unitTy);
	return res;
}

LemniJitResult LemniTypedBoolExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto boolTy = lemniGccjitType(state, this->boolType);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = this->value ? gcc_jit_context_one(state->ctx, boolTy) : gcc_jit_context_zero(state->ctx, boolTy);
	return res;
}

LemniJitResult LemniTypedNat16ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto natTy = gcc_jit_context_get_int_type(state->ctx, 2, 0);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_int(state->ctx, natTy, value);
	return res;
}

LemniJitResult LemniTypedNat32ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto natTy = gcc_jit_context_get_int_type(state->ctx, 4, 0);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_long(state->ctx, natTy, value);
	return res;
}

LemniJitResult LemniTypedNat64ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto natTy = gcc_jit_context_get_int_type(state->ctx, 8, 0);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_long(state->ctx, natTy, value);
	return res;
}

LemniJitResult LemniTypedInt16ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto intTy = gcc_jit_context_get_int_type(state->ctx, 2, 1);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_int(state->ctx, intTy, value);
	return res;
}

LemniJitResult LemniTypedInt32ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto intTy = gcc_jit_context_get_int_type(state->ctx, 4, 1);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_long(state->ctx, intTy, value);
	return res;
}

LemniJitResult LemniTypedInt64ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto intTy = gcc_jit_context_get_int_type(state->ctx, 8, 1);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_long(state->ctx, intTy, value);
	return res;
}

LemniJitResult LemniTypedReal32ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto realTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_FLOAT);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_double(state->ctx, realTy, value);
	return res;
}

LemniJitResult LemniTypedReal64ExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto realTy = gcc_jit_context_get_type(state->ctx, GCC_JIT_TYPE_DOUBLE);

	LemniJitResult res;
	res.hasError = false;
	res.rvalue = gcc_jit_context_new_rvalue_from_double(state->ctx, realTy, value);
	return res;
}

LemniJitResult LemniTypedProductExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	(void)block;

	auto prodTy = lemniGccjitType(state, this->productType);

	(void)prodTy;

	std::vector<gcc_jit_field*> fields;
	std::vector<gcc_jit_rvalue*> rvalues;
	fields.reserve(elems.size());
	rvalues.reserve(elems.size());

	for(std::size_t i = 0; i < elems.size(); i++){
		auto elem = elems[i];

		auto res = elem->compile(state);
		if(res.hasError) return res;

		auto ty = gcc_jit_rvalue_get_type(res.rvalue);

		auto fieldName = "_" + std::to_string(i);

		fields.emplace_back(gcc_jit_context_new_field(state->ctx, nullptr, ty, fieldName.c_str()));
		rvalues.emplace_back(res.rvalue);
	}

	LemniJitResult res;
	res.hasError = true;
	res.error.msg = LEMNICSTR("product literal compilation unimplemented");
	return res;
}

LemniJitResult LemniTypedBranchExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	auto condRes = cond->compile(state, block);
	if(condRes.hasError) return condRes;

	auto fn = gcc_jit_block_get_function(block);

	auto trueBlock = gcc_jit_function_new_block(fn, nullptr);
	auto trueRes = true_->compile(state, trueBlock);
	if(trueRes.hasError) return trueRes;

	auto falseBlock = gcc_jit_function_new_block(fn, nullptr);
	auto falseRes = false_->compile(state, falseBlock);
	if(falseRes.hasError) return falseRes;

	gcc_jit_block_end_with_conditional(block, nullptr, condRes.rvalue, trueBlock, falseBlock);

	LemniJitResult res;
	res.hasError = true;
	res.error.msg = LEMNICSTR("conditional compilation unimplemented");
	return res;
}

LemniJitResult LemniTypedReturnExprT::compile(LemniCompileState state, gcc_jit_block *block) const noexcept{
	auto valRes = this->value->compile(state, block);
	if(valRes.hasError) return valRes;

	gcc_jit_block_end_with_return(block, nullptr, valRes.rvalue);

	return valRes;
}
