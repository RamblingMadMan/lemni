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

#ifndef LEMNI_LIB_LLVM_HPP
#define LEMNI_LIB_LLVM_HPP 1

#include <map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "lemni/Type.h"
#include "lemni/TypedExpr.h"

namespace lemni{
	struct LLVMState{
		LLVMState(const std::string &id)
			: ctx(), module(id, ctx){}

		~LLVMState(){}

		llvm::LLVMContext ctx;
		llvm::Module module;
		std::map<LemniType, llvm::Type*> types;
		std::map<LemniTypedFnDefExpr, llvm::Function*> fns;
		llvm::IRBuilder<> *builder = nullptr;
	};
}

using LemniLLVMState = lemni::LLVMState*;

llvm::Type *lemniMakeLLVMType(LemniLLVMState state, const LemniType type);
llvm::Type *lemniFindLLVMType(LemniLLVMState state, const LemniType type);
llvm::Type *lemniLLVMType(LemniLLVMState state, const LemniType type);

inline llvm::Type *lemniLLVMType(LemniLLVMState state, const LemniType type){
	auto ty = lemniFindLLVMType(state, type);
	if(ty) return ty;

	ty = lemniMakeLLVMType(state, type);

	state->types[type] = ty;

	return ty;
}

inline llvm::Type *lemniFindLLVMType(LemniLLVMState state, const LemniType type){
	auto tyRes = state->types.find(type);
	if(tyRes != end(state->types)) return tyRes->second;
	else return nullptr;
}

inline llvm::Type *lemniMakeLLVMType(LemniLLVMState state, const LemniType type){
	if(lemniTypeAsUnit(type)){
		auto unitTStruct = llvm::StructType::create(state->ctx, "UnitT");
		auto unitTTy = unitTStruct->getPointerTo();
		return unitTTy;
	}
	else if(lemniTypeAsNat(type) || lemniTypeAsInt(type)){
		auto numBits = lemniTypeNumBits(type);

		if((numBits == 0) || (numBits > 64)){
			return nullptr;
		}

		return llvm::Type::getIntNTy(state->ctx, numBits);
	}
	else if(lemniTypeAsRatio(type)){
		auto numBits = lemniTypeNumBits(type);

		if((numBits == 0) || (numBits > 128)){
			return nullptr;
		}

		auto halfBits = numBits / 2;

		std::vector<llvm::Type*> fieldTypes = {
			llvm::Type::getIntNTy(state->ctx, halfBits),
			llvm::Type::getIntNTy(state->ctx, halfBits)
		};

		auto ratioName = "Ratio" + std::to_string(numBits);

		return llvm::StructType::create(state->ctx, fieldTypes, ratioName);
	}
	else if(lemniTypeAsReal(type)){
		auto numBits = lemniTypeNumBits(type);

		switch(numBits){
			case 32: return llvm::Type::getFloatTy(state->ctx);
			case 64: return llvm::Type::getDoubleTy(state->ctx);
			default: return nullptr;
		}
	}
	else if(lemniTypeAsStringUTF8(type)){
		std::vector<llvm::Type*> fieldTypes = {
			llvm::Type::getInt8PtrTy(state->ctx),
			llvm::Type::getIntNTy(state->ctx, 64)
		};

		return llvm::StructType::create(state->ctx, fieldTypes, "StringUTF8");
	}
	else if(lemniTypeAsStringASCII(type)){
		std::vector<llvm::Type*> fieldTypes = {
			llvm::Type::getInt8PtrTy(state->ctx),
			llvm::Type::getIntNTy(state->ctx, 64)
		};

		return llvm::StructType::create(state->ctx, fieldTypes, "StringASCII");
	}
	else if(auto prod = lemniTypeAsProduct(type)){
		auto numComps = lemniProductTypeNumComponents(prod);

		std::vector<llvm::Type*> fieldTypes;
		fieldTypes.reserve(numComps);

		for(std::uint32_t i = 0; i < numComps; i++){
			auto comp = lemniProductTypeComponent(prod, i);
			auto ty = lemniLLVMType(state, comp);
			fieldTypes.emplace_back(ty);
		}

		auto prodMangled = lemni::toStdStr(lemniTypeMangled(type));

		return llvm::StructType::create(state->ctx, fieldTypes, prodMangled);
	}
	else if(auto fn = lemniTypeAsFunction(type)){
		auto retTy = lemniLLVMType(state, lemniFunctionTypeResult(fn));
		auto numParams = lemniFunctionTypeNumParams(fn);

		std::vector<llvm::Type*> paramTypes;
		paramTypes.reserve(numParams);

		for(std::uint32_t i = 0; i < numParams; i++){
			paramTypes.emplace_back(lemniLLVMType(state, lemniFunctionTypeParam(fn, i)));
		}

		return llvm::FunctionType::get(retTy, paramTypes, false);
	}
	else{
		assert(!"type not representable");
	}
}

#endif // !LEMNI_LIB_LLVM_HPP
