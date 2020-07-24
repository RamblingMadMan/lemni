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

#ifndef LEMNI_LIB_TYPEDEXPR_HPP
#define LEMNI_LIB_TYPEDEXPR_HPP 1

#include <map>

#include <ffi.h>
#include <libgccjit.h>

#include "fmt/core.h"

#include "lemni/Macros.h"
#include "lemni/Interop.h"
#include "lemni/AReal.h"
#include "lemni/TypedExpr.h"
#include "lemni/Module.h"
#include "lemni/eval.h"
#include "lemni/compile.h"

#include "Type.hpp"

LEMNI_OPAQUE_T(LemniEvalBindings);

typedef struct LemniJitErrorT{
	LemniStr msg;
} LemniJitError;

typedef struct LemniJitResultT{
	bool hasError;
	union {
		gcc_jit_rvalue *rvalue;
		LemniJitError error;
	};
} LemniJitResult;

struct LemniEvalBindingsT{
	explicit LemniEvalBindingsT(LemniEvalBindingsConst parent_ = nullptr) noexcept
		: parent(parent_){}

	LemniEvalBindingsT(const LemniEvalBindingsT &other)
		: parent(other.parent)
		, bound(other.bound){}

	LemniEvalBindingsT &operator=(const LemniEvalBindingsT &other){
		parent = other.parent;
		bound = other.bound;
		return *this;
	}

	LemniValue find(LemniTypedLValueExpr lval) const{
		auto res = bound.find(lval);
		if(res != end(bound)){
			return res->second.handle();
		}
		else if(parent){
			return parent->find(lval);
		}
		else{
			return nullptr;
		}
	}

	const LemniEvalBindingsT *parent;
	std::map<LemniTypedLValueExpr, lemni::Value> bound;
};


struct LemniTypedExprT{
	virtual ~LemniTypedExprT() = default;

	virtual LemniType type() const noexcept = 0;

	virtual LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept = 0;

	virtual LemniJitResult compile(LemniCompileState state, gcc_jit_block *block = nullptr) const noexcept{
		(void)state;
		(void)block;
		LemniJitResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("compilation unimplemented for expression");
		return res;
	}
};

struct LemniTypedLiteralExprT: LemniTypedExprT{};

struct LemniTypedConstantExprT: LemniTypedLiteralExprT{};

struct LemniTypedModuleExprT: LemniTypedConstantExprT{
	LemniTypedModuleExprT(LemniModuleType moduleType_, LemniModule module_) noexcept
		: moduleType(moduleType_), module(module_)
	{}

	LemniModuleType type() const noexcept override{ return moduleType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniModuleType moduleType;
	LemniModule module;
};

struct LemniTypedTypeExprT: LemniTypedConstantExprT{
	LemniTypedTypeExprT(LemniMetaType metaType_, LemniType value_) noexcept
		: metaType(metaType_), value(value_){}

	LemniMetaType type() const noexcept override{ return metaType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniMetaType metaType;
	LemniType value;
};

struct LemniTypedUnaryOpExprT: LemniTypedExprT{
	LemniTypedUnaryOpExprT(LemniType resultType_, LemniUnaryOp op_, LemniTypedExpr value_) noexcept
		: resultType(resultType_), op(op_), value(value_){}

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniType resultType;
	LemniUnaryOp op;
	LemniTypedExpr value;
};

struct LemniTypedBinaryOpExprT: LemniTypedExprT{
	LemniTypedBinaryOpExprT(LemniType resultType_, LemniBinaryOp op_, LemniTypedExpr lhs_, LemniTypedExpr rhs_) noexcept
		: resultType(resultType_), op(op_), lhs(lhs_), rhs(rhs_){}

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniType resultType;
	LemniBinaryOp op;
	LemniTypedExpr lhs;
	LemniTypedExpr rhs;
};

struct LemniTypedLValueExprT: LemniTypedExprT{
	virtual std::string_view id() const noexcept = 0;
};

struct LemniTypedUnresolvedRefExprT: LemniTypedLValueExprT{
	LemniTypedUnresolvedRefExprT(std::string id_, LemniPseudoType valueType_)
		: m_id(std::move(id_)), valueType(valueType_){}

	std::string_view id() const noexcept override{ return m_id; }

	LemniType type() const noexcept override{ return valueType; }

	LemniEvalResult eval(LemniEvalState, LemniEvalBindings) const noexcept override{
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("unresolved reference");
		return res;
	}

	std::string m_id;
	LemniPseudoType valueType;
};

struct LemniTypedRefExprT: LemniTypedLValueExprT{
	LemniTypedRefExprT(LemniTypedLValueExpr refed_) noexcept
		: refed(refed_){}

	std::string_view id() const noexcept override{ return refed->id(); }

	LemniType type() const noexcept override{ return refed->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override{
		return refed->eval(state, bindings);
	}

	LemniTypedLValueExpr refed;
};

struct LemniTypedNamedExprT: LemniTypedLValueExprT{
	public:
		LemniTypedNamedExprT(std::string id_) noexcept
			: m_id(std::move(id_)){}

		std::string_view id() const noexcept override{ return m_id; }

	private:
		std::string m_id;
};

struct LemniTypedBindingExprT: LemniTypedNamedExprT{
	LemniTypedBindingExprT(std::string id_, LemniTypedExpr value_) noexcept
		: LemniTypedNamedExprT(std::move(id_)), value(value_){}

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniTypedExpr value;
};

struct LemniTypedApplicationExprT: LemniTypedExprT{
	LemniTypedApplicationExprT(LemniType resultType_, LemniTypedExpr fn_, std::vector<LemniTypedExpr> args_) noexcept
		: resultType(resultType_), fn(fn_), args(std::move(args_)){}

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniType resultType;
	LemniTypedExpr fn;
	std::vector<LemniTypedExpr> args;
};

struct LemniTypedProductExprT: LemniTypedLiteralExprT{
	LemniTypedProductExprT(LemniTypeSet types, std::vector<LemniTypedExpr> elems_)
		: productType(nullptr), elems(std::move(elems_))
	{
		std::vector<LemniType> elemTypes;
		elemTypes.reserve(elems.size());

		for(auto elem : elems){
			elemTypes.emplace_back(elem->type());
		}

		productType = lemniTypeSetGetProduct(types, elemTypes.data(), elemTypes.size());
	}

	LemniProductType type() const noexcept override{ return productType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniProductType productType;
	std::vector<LemniTypedExpr> elems;
};

struct LemniTypedBranchExprT: LemniTypedExprT{
	LemniTypedBranchExprT(LemniType resultType_, LemniTypedExpr cond_, LemniTypedExpr true__, LemniTypedExpr false__) noexcept
		: resultType(resultType_), cond(cond_), true_(true__), false_(false__){}

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniType resultType;
	LemniTypedExpr cond, true_, false_;
};

struct LemniTypedReturnExprT: LemniTypedExprT{
	LemniTypedReturnExprT(LemniTypedExpr value_) noexcept
		: value(value_){}

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniTypedExpr value;
};

struct LemniTypedBlockExprT: LemniTypedExprT{
	LemniTypedBlockExprT(LemniType resultType_, std::vector<LemniTypedExpr> exprs_) noexcept
		: resultType(resultType_), exprs(std::move(exprs_)){}

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniType resultType;
	std::vector<LemniTypedExpr> exprs;
};

struct LemniTypedLambdaExprT: LemniTypedLiteralExprT{
	LemniTypedLambdaExprT(LemniFunctionType fnType_, std::vector<LemniTypedExpr> params_, LemniTypedExpr body_) noexcept
		: fnType(fnType_), params(std::move(params_)), body(body_){}

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniFunctionType fnType;
	std::vector<LemniTypedExpr> params;
	LemniTypedExpr body;
};

struct LemniTypedExportExprT: LemniTypedConstantExprT{
	LemniTypedExportExprT(LemniTypedConstantExpr value_) noexcept
		: value(value_){}

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniTypedConstantExpr value;
};

struct LemniTypedUnitExprT: LemniTypedConstantExprT{
	LemniTypedUnitExprT(LemniUnitType unitType_) noexcept
		: unitType(unitType_){}

	LemniUnitType type() const noexcept override{ return unitType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniUnitType unitType;
};

struct LemniTypedBoolExprT: LemniTypedConstantExprT{
	LemniTypedBoolExprT(LemniBoolType boolType_, const bool value_) noexcept
		: boolType(boolType_), value(value_){}

	LemniBoolType type() const noexcept override{ return boolType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniBoolType boolType;
	const bool value;
};

struct LemniTypedNumExprT: LemniTypedConstantExprT{};

struct LemniTypedNatExprT: LemniTypedNumExprT{
	LemniTypedNatExprT(LemniNatType natType_) noexcept
		: natType(natType_){}

	LemniNatType type() const noexcept override{ return natType; }

	LemniNatType natType;
};

struct LemniTypedANatExprT: LemniTypedNatExprT{
	LemniTypedANatExprT(LemniNatType natType_, lemni::AInt value_) noexcept
		: LemniTypedNatExprT(natType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::AInt value;
};

struct LemniTypedNat16ExprT: LemniTypedNatExprT{
	LemniTypedNat16ExprT(LemniNatType nat16Type_, LemniNat16 value_) noexcept
		: LemniTypedNatExprT(nat16Type_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniNat16 value;
};

struct LemniTypedNat32ExprT: LemniTypedNatExprT{
	LemniTypedNat32ExprT(LemniNatType nat32Type_, LemniNat32 value_) noexcept
		: LemniTypedNatExprT(nat32Type_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniNat32 value;
};

struct LemniTypedNat64ExprT: LemniTypedNatExprT{
	LemniTypedNat64ExprT(LemniNatType nat64Type_, LemniNat64 value_) noexcept
		: LemniTypedNatExprT(nat64Type_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniNat64 value;
};

struct LemniTypedIntExprT: LemniTypedNumExprT{
	LemniTypedIntExprT(LemniIntType intType_) noexcept
		: intType(intType_){}

	LemniIntType type() const noexcept override{ return intType; }

	LemniIntType intType;
};

struct LemniTypedAIntExprT: LemniTypedIntExprT{
	LemniTypedAIntExprT(LemniIntType intType_, lemni::AInt value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::AInt value;
};

struct LemniTypedInt16ExprT: LemniTypedIntExprT{
	LemniTypedInt16ExprT(LemniIntType intType_, LemniInt16 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniInt16 value;
};

struct LemniTypedInt32ExprT: LemniTypedIntExprT{
	LemniTypedInt32ExprT(LemniIntType intType_, LemniInt32 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniInt32 value;
};

struct LemniTypedInt64ExprT: LemniTypedIntExprT{
	LemniTypedInt64ExprT(LemniIntType intType_, LemniInt64 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniInt64 value;
};

struct LemniTypedRatioExprT: LemniTypedNumExprT{
	LemniTypedRatioExprT(LemniRatioType ratioType_) noexcept
		: ratioType(ratioType_){}

	LemniRatioType type() const noexcept override{ return ratioType; }

	LemniRatioType ratioType;
};

struct LemniTypedARatioExprT: LemniTypedRatioExprT{
	LemniTypedARatioExprT(LemniRatioType ratioType_, lemni::ARatio value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::ARatio value;
};

struct LemniTypedRatio32ExprT: LemniTypedRatioExprT{
	LemniTypedRatio32ExprT(LemniRatioType ratioType_, LemniRatio32 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniRatio32 value;
};

struct LemniTypedRatio64ExprT: LemniTypedRatioExprT{
	LemniTypedRatio64ExprT(LemniRatioType ratioType_, LemniRatio64 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniRatio64 value;
};

struct LemniTypedRatio128ExprT: LemniTypedRatioExprT{
	LemniTypedRatio128ExprT(LemniRatioType ratioType_, LemniRatio128 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniRatio128 value;
};

struct LemniTypedRealExprT: LemniTypedNumExprT{
	explicit LemniTypedRealExprT(LemniRealType realType_) noexcept
		: realType(realType_){}

	LemniRealType type() const noexcept override{ return realType; }

	LemniRealType realType;
};

struct LemniTypedARealExprT: LemniTypedRealExprT{
	LemniTypedARealExprT(LemniRealType realType_, lemni::AReal value_) noexcept
		: LemniTypedRealExprT(realType_), value(std::move(value_)){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::AReal value;
};

struct LemniTypedReal32ExprT: LemniTypedRealExprT{
	LemniTypedReal32ExprT(LemniRealType real32Type_, float value_) noexcept
		: LemniTypedRealExprT(real32Type_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block = nullptr) const noexcept override;

	LemniReal32 value;
};

struct LemniTypedReal64ExprT: LemniTypedRealExprT{
	LemniTypedReal64ExprT(LemniRealType real64Type_, float value_) noexcept
		: LemniTypedRealExprT(real64Type_), value(value_){}

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, gcc_jit_block *block) const noexcept override;

	LemniReal64 value;
};

struct LemniTypedStringExprT: LemniTypedConstantExprT{
	virtual std::string_view str() const noexcept = 0;
};

struct LemniTypedStringASCIIExprT: LemniTypedStringExprT{
	LemniTypedStringASCIIExprT(LemniStringASCIIType strType_, std::string value_) noexcept
		: strType(strType_), value(std::move(value_)){}

	LemniStringASCIIType type() const noexcept override{ return strType; }
	std::string_view str() const noexcept override{ return value; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniStringASCIIType strType;
	std::string value;
};

struct LemniTypedStringUTF8ExprT: LemniTypedStringExprT{
	LemniTypedStringUTF8ExprT(LemniStringUTF8Type strType_, std::string value_) noexcept
		: strType(strType_), value(std::move(value_)){}

	LemniStringUTF8Type type() const noexcept override{ return strType; }
	std::string_view str() const noexcept override{ return value; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniStringUTF8Type strType;
	std::string value;
};

struct LemniTypedParamBindingExprT: LemniTypedNamedExprT{
	LemniTypedParamBindingExprT(std::string id_, LemniType valueType_) noexcept
		: LemniTypedNamedExprT(std::move(id_)), valueType(valueType_){}

	LemniType type() const noexcept override{ return valueType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniType valueType;
};

struct LemniTypedFnDefExprT: LemniTypedNamedExprT{
	LemniTypedFnDefExprT(LemniTypeSet types, std::string id_, LemniType resultType, std::vector<LemniTypedParamBindingExpr> params_, LemniTypedExpr body_)
		: LemniTypedNamedExprT(std::move(id_)), params(std::move(params_)), body(body_)
	{
		std::vector<LemniType> paramTypes;
		paramTypes.reserve(params.size());

		for(auto p : params){
			paramTypes.emplace_back(p->valueType);
		}

		fnType = lemniTypeSetGetFunction(types, resultType, paramTypes.data(), paramTypes.size());
	}

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	std::vector<LemniTypedParamBindingExpr> params;
	LemniTypedExpr body;
	LemniFunctionType fnType;
};

ffi_type *lemniTypeToFFI(LemniType type);

struct LemniTypedExtFnDeclExprT: LemniTypedNamedExprT{
	LemniTypedExtFnDeclExprT(LemniTypeSet types, std::string name, void *const ptr_, LemniType resultType, std::vector<std::pair<std::string, LemniType>> params)
		: LemniTypedNamedExprT(std::move(name)), ptr(ptr_)
	{
		auto resultFFIType = lemniTypeToFFI(resultType);
		if(!resultFFIType){
			throw std::runtime_error("could not convert result type for FFI");
		}

		std::vector<LemniType> paramTypes;
		std::vector<ffi_type*> paramFFITypes;
		paramNames.reserve(params.size());
		paramTypes.reserve(params.size());
		paramFFITypes.reserve(params.size());

		for(auto p : params){
			paramNames.emplace_back(std::move(p.first));
			paramTypes.emplace_back(p.second);

			auto ffiType = lemniTypeToFFI(p.second);
			if(!ffiType){
				throw std::runtime_error("could not convert type for FFI");
			}

			paramFFITypes.emplace_back(ffiType);
		}

		if(params.empty()){
			paramTypes.emplace_back(lemniTypeSetGetUnit(types));
		}

		fnType = lemniTypeSetGetFunction(types, resultType, paramTypes.data(), paramTypes.size());

		auto ffiRes = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, params.size(), resultFFIType, paramFFITypes.data());
		if(ffiRes != FFI_OK){
			switch(ffiRes){
				case FFI_BAD_TYPEDEF:{
					throw std::runtime_error("error in ffi_prep_cif: FFI_BAD_TYPEDEF");
				}

				case FFI_BAD_ABI:{
					throw std::runtime_error("error in ffi_prep_cif: FFI_BAD_ABI");
				}

				default:{
					throw std::runtime_error("error in ffi_prep_cif: " + std::to_string(ffiRes));
				}
			}
		}
	}

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	void *ptr;
	std::vector<std::string> paramNames;
	LemniFunctionType fnType;
	mutable ffi_cif cif;
};

#endif // !LEMNI_LIB_TYPEDEXPR_HPP
