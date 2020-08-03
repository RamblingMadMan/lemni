#ifndef LEMNI_LIB_GCCJIT_HPP
#define LEMNI_LIB_GCCJIT_HPP 1

#include <memory>
#include <vector>
#include <map>

#include <libgccjit.h>

#include "lemni/Type.h"
#include "lemni/TypedExpr.h"

namespace lemni{
	struct GCCJITState{
		explicit GCCJITState(gcc_jit_context *ctxParent = nullptr){
			if(ctxParent){
				ctx = gcc_jit_context_new_child_context(ctxParent);
			}
			else{
				ctx = gcc_jit_context_acquire();
			}
		}

		~GCCJITState(){
			gcc_jit_context_release(ctx);
		}

		gcc_jit_context *ctx;

		std::map<LemniType, gcc_jit_type*> types;
		std::map<LemniTypedFnDefExpr, gcc_jit_function*> fns;
	};
}

using LemniGCCJITState = lemni::GCCJITState*;

gcc_jit_type *lemniMakeGccjitType(LemniGCCJITState state, const LemniType type);
gcc_jit_type *lemniFindGccjitType(LemniGCCJITState state, const LemniType type);

inline gcc_jit_type *lemniGccjitType(LemniGCCJITState state, const LemniType type){
	auto ty = lemniFindGccjitType(state, type);
	if(ty) return ty;

	ty = lemniMakeGccjitType(state, type);

	state->types[type] = ty;

	return ty;
}

inline gcc_jit_type *lemniFindGccjitType(LemniGCCJITState state, const LemniType type){
	auto tyRes = state->types.find(type);
	if(tyRes != end(state->types)) return tyRes->second;
	else return nullptr;
}

inline gcc_jit_type *lemniMakeGccjitType(LemniGCCJITState state, const LemniType type){
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

#endif // !LEMNI_LIB_GCCJIT_HPP
