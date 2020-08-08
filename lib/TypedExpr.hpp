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

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"

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

namespace {
	template<typename ExprT, typename ... Args>
	inline ExprT *newTypedExpr(Args &&... args) noexcept(noexcept(ExprT(std::forward<Args>(args)...))){
		auto mem = std::malloc(sizeof(ExprT));
		if(!mem) return nullptr;
		return new(mem) ExprT(std::forward<Args>(args)...);
	}

	inline void deleteTypedExpr(LemniTypedExpr expr) noexcept{
		std::destroy_at(expr);
		std::free(const_cast<LemniTypedExprT*>(expr)); // ew, plz C
	}
}

typedef struct LemniJitErrorT{
	LemniStr msg;
} LemniJitError;

typedef struct LemniJitResultT{
	bool hasError;
	union {
		llvm::Value *rvalue;
		LemniJitError error;
	};
} LemniJitResult;

typedef struct LemniCompileContextT *LemniCompileContext;

struct LemniCompileContextT{
	LemniCompileContext parent = nullptr;
	LemniTypedFnDefExpr def = nullptr;
	llvm::Function *fn = nullptr;
	llvm::IRBuilder<> *builder = nullptr;
};

typedef struct LemniPartialBindingsT *LemniPartialBindings;
typedef const struct LemniPartialBindingsT *LemniPartialBindingsConst;

struct LemniPartialBindingsT{
	explicit LemniPartialBindingsT(LemniPartialBindingsConst parent_ = nullptr) noexcept
		: parent(parent_){}

	LemniTypedExpr find(LemniTypedExpr expr) const{
		auto res = bound.find(expr);
		if(res != end(bound)){
			return res->second;
		}
		else if(parent){
			return parent->find(expr);
		}
		else{
			return nullptr;
		}
	}

	LemniPartialBindingsConst parent;
	std::map<LemniTypedExpr, LemniTypedExpr> bound;
};

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

	virtual LemniTypedExpr clone() const noexcept = 0;

	virtual LemniType type() const noexcept = 0;

	virtual LemniStr toStr() const noexcept{
		return { .ptr = nullptr, .len = 0 };
	}

	virtual LemniTypedExpr deref() const noexcept{ return this; }

	virtual LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept;

	virtual LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept = 0;

	virtual LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx = nullptr) const noexcept{
		(void)state;
		(void)ctx;
		LemniJitResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("compilation unimplemented for expression");
		return res;
	}
};

struct LemniTypedLiteralExprT: LemniTypedExprT{};

struct LemniTypedMacroExprT: LemniTypedLiteralExprT{
	LemniTypedMacroExprT(LemniExprType exprType_, std::vector<LemniExpr> exprs_) noexcept
		: exprType(exprType_), exprs(std::move(exprs_)){}

	LemniTypedMacroExpr clone() const noexcept override{
		return newTypedExpr<LemniTypedMacroExprT>(exprType, exprs);
	}

	LemniExprType type() const noexcept override{ return exprType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override{
		(void)state;
		(void)bindings;
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("macro expression evaluation unimplemented");
		return res;
	}

	LemniExprType exprType;
	std::vector<LemniExpr> exprs;
};

struct LemniTypedPlaceholderExprT: LemniTypedLiteralExprT{
	LemniTypedPlaceholderExprT(LemniPseudoType pseudoType_) noexcept
		: pseudoType(pseudoType_){}

	LemniTypedPlaceholderExpr clone() const noexcept  override{
		return newTypedExpr<LemniTypedPlaceholderExprT>(pseudoType);
	}

	LemniPseudoType type() const noexcept override{ return pseudoType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override{
		(void)state;
		(void)bindings;
		LemniEvalResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("can not evaluate placeholder expression");
		return res;
	}

	LemniPseudoType pseudoType;
};

struct LemniTypedConstantExprT: LemniTypedLiteralExprT{};

struct LemniTypedModuleExprT: LemniTypedConstantExprT{
	LemniTypedModuleExprT(LemniModuleType moduleType_, LemniModule module_) noexcept
		: moduleType(moduleType_), module(module_)
	{}

	LemniTypedModuleExpr clone() const noexcept override{
		return newTypedExpr<LemniTypedModuleExprT>(moduleType, module);
	}

	LemniModuleType type() const noexcept override{ return moduleType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniModuleType moduleType;
	LemniModule module;
};

struct LemniTypedTypeExprT: LemniTypedConstantExprT{
	LemniTypedTypeExprT(LemniMetaType metaType_, LemniType value_) noexcept
		: metaType(metaType_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedTypeExprT>(metaType, value); }

	LemniMetaType type() const noexcept override{ return metaType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniMetaType metaType;
	LemniType value;
};

struct LemniTypedUnaryOpExprT: LemniTypedExprT{
	LemniTypedUnaryOpExprT(LemniType resultType_, LemniUnaryOp op_, LemniTypedExpr value_) noexcept
		: resultType(resultType_), op(op_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedUnaryOpExprT>(resultType, op, value); }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override;

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniType resultType;
	LemniUnaryOp op;
	LemniTypedExpr value;
};

struct LemniTypedBinaryOpExprT: LemniTypedExprT{
	LemniTypedBinaryOpExprT(LemniType resultType_, LemniBinaryOp op_, LemniTypedExpr lhs_, LemniTypedExpr rhs_) noexcept
		: resultType(resultType_), op(op_), lhs(lhs_), rhs(rhs_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedBinaryOpExprT>(resultType, op, lhs, rhs); }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override;

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

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

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedUnresolvedRefExprT>(m_id, valueType); }

	std::string_view id() const noexcept override{ return m_id; }

	LemniType type() const noexcept override{ return valueType; }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override;

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

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedRefExprT>(refed); }

	std::string_view id() const noexcept override{ return refed->id(); }

	LemniType type() const noexcept override{ return refed->type(); }

	LemniTypedExpr deref() const noexcept override{ return refed->deref(); }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override{
		return refed->partialEval(state, bindings, numArgs, args);
	}

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

	protected:
		std::string m_id;
};

struct LemniTypedBindingExprT: LemniTypedNamedExprT{
	LemniTypedBindingExprT(std::string id_, LemniTypedExpr value_) noexcept
		: LemniTypedNamedExprT(std::move(id_)), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedBindingExprT>(m_id, value); }

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniTypedExpr value;
};

struct LemniTypedApplicationExprT: LemniTypedExprT{
	LemniTypedApplicationExprT(LemniType resultType_, LemniTypedExpr fn_, std::vector<LemniTypedExpr> args_) noexcept
		: resultType(resultType_), fn(fn_), args(std::move(args_)){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedApplicationExprT>(resultType, fn, args); }

	LemniType type() const noexcept override{ return resultType; }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args_) const noexcept override;

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

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

		isConstant = true;

		for(auto elem : elems){
			elemTypes.emplace_back(elem->type());
			if(isConstant && !dynamic_cast<LemniTypedConstantExpr>(elem)){
				isConstant = false;
			}
		}

		productType = lemniTypeSetGetProduct(types, elemTypes.data(), elemTypes.size());
	}

	LemniTypedProductExprT(LemniProductType productType_, std::vector<LemniTypedExpr> elems_, bool isConstant_)
		: productType(productType_), elems(std::move(elems_)), isConstant(isConstant_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedProductExprT>(productType, elems, isConstant); }

	LemniProductType type() const noexcept override{ return productType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniProductType productType;
	std::vector<LemniTypedExpr> elems;
	bool isConstant = false;
};

struct LemniTypedBranchExprT: LemniTypedExprT{
	LemniTypedBranchExprT(LemniType resultType_, LemniTypedExpr cond_, LemniTypedExpr true__, LemniTypedExpr false__) noexcept
		: resultType(resultType_), cond(cond_), true_(true__), false_(false__){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedBranchExprT>(resultType, cond, true_, false_); }

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniType resultType;
	LemniTypedExpr cond, true_, false_;
};

struct LemniTypedReturnExprT: LemniTypedExprT{
	LemniTypedReturnExprT(LemniTypedExpr value_) noexcept
		: value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedReturnExprT>(value); }

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniTypedExpr value;
};

struct LemniTypedBlockExprT: LemniTypedExprT{
	LemniTypedBlockExprT(LemniType resultType_, std::vector<LemniTypedExpr> exprs_) noexcept
		: resultType(resultType_), exprs(std::move(exprs_)){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedBlockExprT>(resultType, exprs); }

	LemniType type() const noexcept override{ return resultType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniType resultType;
	std::vector<LemniTypedExpr> exprs;
};

struct LemniTypedLambdaExprT: LemniTypedLiteralExprT{
	LemniTypedLambdaExprT(LemniFunctionType fnType_, std::vector<LemniTypedExpr> params_, LemniTypedExpr body_) noexcept
		: fnType(fnType_), params(std::move(params_)), body(body_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedLambdaExprT>(fnType, params, body); }

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniFunctionType fnType;
	std::vector<LemniTypedExpr> params;
	LemniTypedExpr body;
};

struct LemniTypedExportExprT: LemniTypedConstantExprT{
	LemniTypedExportExprT(LemniTypedConstantExpr value_) noexcept
		: value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedExportExprT>(value); }

	LemniType type() const noexcept override{ return value->type(); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniTypedConstantExpr value;
};

struct LemniTypedUnitExprT: LemniTypedConstantExprT{
	LemniTypedUnitExprT(LemniUnitType unitType_) noexcept
		: unitType(unitType_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedUnitExprT>(unitType); }

	LemniUnitType type() const noexcept override{ return unitType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniUnitType unitType;
};

struct LemniTypedBoolExprT: LemniTypedConstantExprT{
	LemniTypedBoolExprT(LemniBoolType boolType_, const bool value_) noexcept
		: boolType(boolType_), value(value_ ? LEMNI_TRUE : LEMNI_FALSE){}

	LemniTypedBoolExprT(LemniBoolType boolType_, const LemniBool value_) noexcept
		: boolType(boolType_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedBoolExprT>(boolType, value); }

	LemniBoolType type() const noexcept override{ return boolType; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniBoolType boolType;
	const LemniBool value;
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

	LemniTypedANatExpr clone() const noexcept override{ return newTypedExpr<LemniTypedANatExprT>(natType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::AInt value;
};

struct LemniTypedNatNExprT: LemniTypedNatExprT{
	LemniTypedNatNExprT(LemniNatType natType_, LemniNat64 numBits_, std::vector<LemniNat64> bits_) noexcept
		: LemniTypedNatExprT(natType_), numBits(numBits_), bits(std::move(bits_)){}

	LemniTypedNatNExprT(LemniNatType natType_, LemniNat64 numBits_, LemniNat64 bits_) noexcept
		: LemniTypedNatExprT(natType_), numBits(numBits_), bits{ bits_ }{}

	LemniTypedNatExpr clone() const noexcept override{ return newTypedExpr<LemniTypedNatNExprT>(natType, numBits, bits); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniNat64 numBits;
	std::vector<LemniNat64> bits;
};

struct LemniTypedNat16ExprT: LemniTypedNatExprT{
	LemniTypedNat16ExprT(LemniNatType nat16Type_, LemniNat16 value_) noexcept
		: LemniTypedNatExprT(nat16Type_), value(value_){}

	LemniTypedNat16Expr clone() const noexcept override{ return newTypedExpr<LemniTypedNat16ExprT>(natType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniNat16 value;
};

struct LemniTypedNat32ExprT: LemniTypedNatExprT{
	LemniTypedNat32ExprT(LemniNatType nat32Type_, LemniNat32 value_) noexcept
		: LemniTypedNatExprT(nat32Type_), value(value_){}

	LemniTypedNat32Expr clone() const noexcept override{ return newTypedExpr<LemniTypedNat32ExprT>(natType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniNat32 value;
};

struct LemniTypedNat64ExprT: LemniTypedNatExprT{
	LemniTypedNat64ExprT(LemniNatType nat64Type_, LemniNat64 value_) noexcept
		: LemniTypedNatExprT(nat64Type_), value(value_){}

	LemniTypedNat64Expr clone() const noexcept override{ return newTypedExpr<LemniTypedNat64ExprT>(natType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

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

	LemniTypedAIntExpr clone() const noexcept override{ return newTypedExpr<LemniTypedAIntExprT>(intType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	//LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	lemni::AInt value;
};

struct LemniTypedIntNExprT: LemniTypedIntExprT{
	LemniTypedIntNExprT(LemniIntType intType_, LemniNat64 numBits_, std::vector<LemniNat64> bits_) noexcept
		: LemniTypedIntExprT(intType_), numBits(numBits_), bits(std::move(bits_)){}

	LemniTypedIntNExprT(LemniIntType intType_, LemniNat64 numBits_, LemniNat64 bits_) noexcept
		: LemniTypedIntExprT(intType_), numBits(numBits_), bits{ bits_ }{}

	LemniTypedIntExpr clone() const noexcept override{ return newTypedExpr<LemniTypedIntNExprT>(intType, numBits, bits); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniNat64 numBits;
	std::vector<LemniNat64> bits;
};

struct LemniTypedInt16ExprT: LemniTypedIntExprT{
	LemniTypedInt16ExprT(LemniIntType intType_, LemniInt16 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniTypedInt16Expr clone() const noexcept override{ return newTypedExpr<LemniTypedInt16ExprT>(intType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniInt16 value;
};

struct LemniTypedInt32ExprT: LemniTypedIntExprT{
	LemniTypedInt32ExprT(LemniIntType intType_, LemniInt32 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniTypedInt32Expr clone() const noexcept override{ return newTypedExpr<LemniTypedInt32ExprT>(intType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniInt32 value;
};

struct LemniTypedInt64ExprT: LemniTypedIntExprT{
	LemniTypedInt64ExprT(LemniIntType intType_, LemniInt64 value_) noexcept
		: LemniTypedIntExprT(intType_), value(std::move(value_)){}

	LemniTypedInt64Expr clone() const noexcept override{ return newTypedExpr<LemniTypedInt64ExprT>(intType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

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

	LemniTypedARatioExpr clone() const noexcept override{ return newTypedExpr<LemniTypedARatioExprT>(ratioType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::ARatio value;
};

struct LemniTypedRatio32ExprT: LemniTypedRatioExprT{
	LemniTypedRatio32ExprT(LemniRatioType ratioType_, LemniRatio32 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedRatio32ExprT>(ratioType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniRatio32 value;
};

struct LemniTypedRatio64ExprT: LemniTypedRatioExprT{
	LemniTypedRatio64ExprT(LemniRatioType ratioType_, LemniRatio64 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedRatio64ExprT>(ratioType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniRatio64 value;
};

struct LemniTypedRatio128ExprT: LemniTypedRatioExprT{
	LemniTypedRatio128ExprT(LemniRatioType ratioType_, LemniRatio128 value_) noexcept
		: LemniTypedRatioExprT(ratioType_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedRatio128ExprT>(ratioType, value); }

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

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedARealExprT>(realType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	lemni::AReal value;
};

struct LemniTypedReal32ExprT: LemniTypedRealExprT{
	LemniTypedReal32ExprT(LemniRealType real32Type_, float value_) noexcept
		: LemniTypedRealExprT(real32Type_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedReal32ExprT>(realType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx = nullptr) const noexcept override;

	LemniReal32 value;
};

struct LemniTypedReal64ExprT: LemniTypedRealExprT{
	LemniTypedReal64ExprT(LemniRealType real64Type_, float value_) noexcept
		: LemniTypedRealExprT(real64Type_), value(value_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedReal64ExprT>(realType, value); }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniJitResult compile(LemniCompileState state, LemniCompileContext ctx) const noexcept override;

	LemniReal64 value;
};

struct LemniTypedStringExprT: LemniTypedConstantExprT{
	virtual std::string_view str() const noexcept = 0;
};

struct LemniTypedStringASCIIExprT: LemniTypedStringExprT{
	LemniTypedStringASCIIExprT(LemniStringASCIIType strType_, std::string value_) noexcept
		: strType(strType_), value(std::move(value_)){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedStringASCIIExprT>(strType, value); }

	LemniStringASCIIType type() const noexcept override{ return strType; }
	std::string_view str() const noexcept override{ return value; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniStringASCIIType strType;
	std::string value;
};

struct LemniTypedStringUTF8ExprT: LemniTypedStringExprT{
	LemniTypedStringUTF8ExprT(LemniStringUTF8Type strType_, std::string value_) noexcept
		: strType(strType_), value(std::move(value_)){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedStringUTF8ExprT>(strType, value); }

	LemniStringUTF8Type type() const noexcept override{ return strType; }
	std::string_view str() const noexcept override{ return value; }

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	LemniStringUTF8Type strType;
	std::string value;
};

struct LemniTypedParamBindingExprT: LemniTypedNamedExprT{
	LemniTypedParamBindingExprT(std::string id_, LemniType valueType_) noexcept
		: LemniTypedNamedExprT(std::move(id_)), valueType(valueType_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedParamBindingExprT>(m_id, valueType); }

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

	LemniTypedFnDefExprT(std::string id_, LemniFunctionType fnType_, std::vector<LemniTypedParamBindingExpr> params_, LemniTypedExpr body_)
		: LemniTypedNamedExprT(std::move(id_)), params(std::move(params_)), body(body_), fnType(fnType_){}

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedFnDefExprT>(m_id, fnType, params, body); }

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override;

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

	LemniTypedExtFnDeclExprT(LemniFunctionType fnType_, std::string name_, void *const ptr_, std::vector<std::string> paramNames_)
		: LemniTypedNamedExprT(std::move(name_)), ptr(ptr_), paramNames(std::move(paramNames_)), fnType(fnType_)
	{
		auto resultType = lemniFunctionTypeResult(fnType);
		auto resultFFIType = lemniTypeToFFI(resultType);
		if(!resultFFIType){
			throw std::runtime_error("could not convert result type for FFI");
		}

		auto numParams = lemniFunctionTypeNumParams(fnType);

		std::vector<ffi_type*> paramFFITypes;
		paramFFITypes.reserve(numParams);

		for(LemniNat64 i = 0; i < numParams; i++){
			auto paramType = lemniFunctionTypeParam(fnType, i);
			auto paramFFIType = lemniTypeToFFI(paramType);
			if(!paramFFIType){
				throw std::runtime_error("could not convert type for FFI");
			}

			paramFFITypes.emplace_back(paramFFIType);
		}

		auto ffiRes = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, numParams, resultFFIType, paramFFITypes.data());
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

	LemniTypedExpr clone() const noexcept override{ return newTypedExpr<LemniTypedExtFnDeclExprT>(fnType, m_id, ptr, paramNames); }

	LemniFunctionType type() const noexcept override{ return fnType; }

	LemniTypecheckResult partialEval(LemniTypecheckState state, LemniPartialBindings bindings, const LemniNat64 numArgs, LemniTypedExpr *const args) const noexcept override;

	LemniEvalResult eval(LemniEvalState state, LemniEvalBindings bindings) const noexcept override;

	void *ptr;
	std::vector<std::string> paramNames;
	LemniFunctionType fnType;
	mutable ffi_cif cif;
};

#endif // !LEMNI_LIB_TYPEDEXPR_HPP
