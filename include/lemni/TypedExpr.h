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

#ifndef LEMNI_TYPEDEXPR_H
#define LEMNI_TYPEDEXPR_H 1

#include "Location.h"
#include "Str.h"
#include "Type.h"
#include "Operator.h"
#include "AReal.h"

/**
 * @defgroup TypedExprs Types and functions related to typed expressions.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

LEMNI_OPAQUE_CONST_T(LemniTypedExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedUnaryOpExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedBinaryOpExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedApplicationExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedLiteralExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedMacroExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedPlaceholderExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedProductExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedLambdaExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedConstantExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedExportExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedModuleExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedUnitExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedNumExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedNatExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedANatExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedNat16Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedNat32Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedNat64Expr);

LEMNI_OPAQUE_CONST_T(LemniTypedIntExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedAIntExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedInt16Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedInt32Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedInt64Expr);

LEMNI_OPAQUE_CONST_T(LemniTypedRatioExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedARatioExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedRatio32Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedRatio64Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedRatio128Expr);

LEMNI_OPAQUE_CONST_T(LemniTypedRealExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedARealExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedReal32Expr);
LEMNI_OPAQUE_CONST_T(LemniTypedReal64Expr);

LEMNI_OPAQUE_CONST_T(LemniTypedStringExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedStringASCIIExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedStringUTF8Expr);

LEMNI_OPAQUE_CONST_T(LemniTypedTypeExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedRefExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedLValueExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedBindingExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedParamBindingExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedFnDefExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedExtFnDeclExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedBlockExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedReturnExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedBranchExpr);


typedef struct {
	bool isClosure;
	union {
		LemniFunctionType function;
		LemniClosureType closure;
	};
} LemniTypedFnExprType;

/**
 * @brief Destroy a typed expression previously created with a ``lemniCreateTyped*`` function.
 * @warning this function should not be used on typed expressions created by typechecking.
 * @param expr typed expression to destroy
 */
void lemniDestroyTypedExpr(LemniTypedExpr expr);

LemniStr lemniTypedExprStr(LemniTypedExpr expr);
LemniType lemniTypedExprType(LemniTypedExpr expr);

LemniTypedUnaryOpExpr lemniCreateTypedUnaryOp(LemniUnaryOp op, LemniTypedExpr value);
LemniTypedUnaryOpExpr lemniTypedExprAsUnaryOp(LemniTypedExpr expr);
LemniTypedExpr lemniTypedUnaryOpExprBase(LemniTypedUnaryOpExpr unaryOp);
LemniType lemniTypedUnaryOpExprType(LemniTypedUnaryOpExpr unaryOp);
LemniTypedExpr lemniTypedUnaryOpExprValue(LemniTypedUnaryOpExpr unaryOp);
LemniUnaryOp lemniTypedUnaryOpExprOp(LemniTypedUnaryOpExpr unaryOp);

LemniTypedBinaryOpExpr lemniCreateTypedBinaryOp(LemniBinaryOp op, LemniTypedExpr lhs, LemniTypedExpr rhs);
LemniTypedBinaryOpExpr lemniTypedExprAsBinaryOp(LemniTypedExpr expr);
LemniTypedExpr lemniTypedBinaryOpExprBase(LemniTypedBinaryOpExpr binaryOp);
LemniType lemniTypedBinaryOpExprType(LemniTypedBinaryOpExpr binaryOp);
LemniTypedExpr lemniTypedBinaryOpExprLhs(LemniTypedBinaryOpExpr binaryOp);
LemniTypedExpr lemniTypedBinaryOpExprRhs(LemniTypedBinaryOpExpr binaryOp);
LemniBinaryOp lemniTypedBinaryOpExprOp(LemniTypedBinaryOpExpr binaryOp);

LemniTypedApplicationExpr lemniCreateTypedApplication(LemniTypedExpr fn, LemniTypedExpr *const args, const uint32_t numArgs);
LemniTypedApplicationExpr lemniTypedExprAsApplication(LemniTypedExpr expr);
LemniTypedExpr lemniTypedApplicationExprBase(LemniTypedApplicationExpr application);
LemniType lemniTypedApplicationExprType(LemniTypedApplicationExpr application);
LemniTypedExpr lemniTypedApplicationExprFunctor(LemniTypedApplicationExpr application);
uint32_t lemniTypedApplicationExprNumArgs(LemniTypedApplicationExpr application);
LemniTypedExpr lemniTypedApplicationExprArg(LemniTypedApplicationExpr application, const uint32_t idx);

LemniTypedLiteralExpr lemniTypedExprAsLiteral(LemniTypedExpr expr);
LemniTypedExpr lemniTypedLiteralExprBase(LemniTypedLiteralExpr lit);
LemniType lemniTypedLiteralExprType(LemniTypedLiteralExpr lit);

LemniTypedLambdaExpr lemniCreateTypedLambda(
	LemniTypedExpr *const closed, const uint32_t numClosed,
	LemniTypedExpr *const params, const uint32_t numParams,
	LemniTypedExpr body
);
LemniTypedLambdaExpr lemniTypedLiteralExprAsLambda(LemniTypedLiteralExpr lit);
LemniTypedLiteralExpr lemniTypedLambdaExprBase(LemniTypedLambdaExpr lambda);
LemniTypedFnExprType lemniTypedLambdaExprType(LemniTypedLambdaExpr lambda);
uint32_t lemniTypedLambdaExprNumParams(LemniTypedLambdaExpr lambda);
LemniTypedExpr lemniTypedLambdaExprParam(LemniTypedLambdaExpr lambda, const uint32_t idx);
LemniTypedExpr lemniTypedLambdaExprBody(LemniTypedLambdaExpr lambda);

/***
 *  Constant literal expressions
 ***/

LemniTypedConstantExpr lemniTypedLiteralAsConstant(LemniTypedLiteralExpr lit);
LemniType lemniTypedConstantType(LemniTypedConstantExpr constant);
LemniTypedLiteralExpr lemniTypedConstantBase(LemniTypedConstantExpr constant);
LemniTypedExpr lemniTypedConstantRoot(LemniTypedConstantExpr constant);

LemniTypedNumExpr lemniTypedConstantAsNum(LemniTypedConstantExpr constant);
LemniType lemniTypedNumType(LemniTypedNumExpr num);
LemniTypedConstantExpr lemniTypedNumBase(LemniTypedNumExpr num);
LemniTypedExpr lemniTypedNumRoot(LemniTypedNumExpr num);

/***
 * Natural literal expressions
 ***/

LemniTypedNatExpr lemniTypedNumAsNat(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedNatBase(LemniTypedNatExpr n);
LemniNatType lemniTypedNatType(LemniTypedNatExpr n);

LemniTypedANatExpr lemniCreateTypedANat(LemniTypeSet types, LemniAIntConst val);
LemniTypedNat16Expr lemniCreateTypedNat16(LemniTypeSet types, LemniNat16 val);
LemniTypedNat32Expr lemniCreateTypedNat32(LemniTypeSet types, LemniNat32 val);
LemniTypedNat64Expr lemniCreateTypedNat64(LemniTypeSet types, LemniNat64 val);

LemniAIntConst lemniTypedANatValue(LemniTypedANatExpr n);
LemniNat16 lemniTypedNat16Value(LemniTypedNat16Expr n16);
LemniNat32 lemniTypedNat32Value(LemniTypedNat32Expr n32);
LemniNat64 lemniTypedNat64Value(LemniTypedNat64Expr n64);

LemniTypedNatExpr lemniTypedANatBase(LemniTypedANatExpr n);
LemniTypedNatExpr lemniTypedNat16Base(LemniTypedNat16Expr n16);
LemniTypedNatExpr lemniTypedNat32Base(LemniTypedNat32Expr n32);
LemniTypedNatExpr lemniTypedNat64Base(LemniTypedNat64Expr n64);

LemniTypedExpr lemniTypedANatRoot(LemniTypedANatExpr n);
LemniTypedExpr lemniTypedNat16Root(LemniTypedNat16Expr n16);
LemniTypedExpr lemniTypedNat32Root(LemniTypedNat32Expr n32);
LemniTypedExpr lemniTypedNat64Root(LemniTypedNat64Expr n64);

/***
 *  Integer literal expressions
 ***/

LemniTypedIntExpr lemniTypedNumAsInt(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedIntBase(LemniTypedIntExpr nat);
LemniIntType lemniTypedIntType(LemniTypedIntExpr nat);

LemniTypedAIntExpr lemniCreateTypedAInt(LemniTypeSet types, LemniAIntConst val);
LemniTypedInt16Expr lemniCreateTypedInt16(LemniTypeSet types, LemniInt16 val);
LemniTypedInt32Expr lemniCreateTypedInt32(LemniTypeSet types, LemniInt32 val);
LemniTypedInt64Expr lemniCreateTypedInt64(LemniTypeSet types, LemniInt64 val);

LemniTypedAIntExpr lemniTypedIntAsAInt(LemniTypedIntExpr z);
LemniTypedInt16Expr lemniTypedIntAsInt16(LemniTypedIntExpr z);
LemniTypedInt32Expr lemniTypedIntAsInt32(LemniTypedIntExpr z);
LemniTypedInt64Expr lemniTypedIntAsInt64(LemniTypedIntExpr z);

LemniAIntConst lemniTypedAIntValue(LemniTypedAIntExpr z);
LemniInt16 lemniTypedInt16Value(LemniTypedInt16Expr z16);
LemniInt32 lemniTypedInt32Value(LemniTypedInt32Expr z32);
LemniInt64 lemniTypedInt64Value(LemniTypedInt64Expr z64);

LemniTypedIntExpr lemniTypedAIntBase(LemniTypedAIntExpr z);
LemniTypedIntExpr lemniTypedInt16Base(LemniTypedInt16Expr z16);
LemniTypedIntExpr lemniTypedInt32Base(LemniTypedInt32Expr z32);
LemniTypedIntExpr lemniTypedInt64Base(LemniTypedInt64Expr z64);

/***
 *  Rational literal expressions
 ***/

LemniTypedRatioExpr lemniTypedNumAsRatio(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRatioBase(LemniTypedRatioExpr q);
LemniRatioType lemniTypedRatioType(LemniTypedRatioExpr q);

LemniTypedARatioExpr lemniCreateTypedARatio(LemniTypeSet types, LemniARatioConst val);
LemniTypedRatio32Expr lemniCreateTypedRatio32(LemniTypeSet types, LemniRatio32 val);
LemniTypedRatio64Expr lemniCreateTypedRatio64(LemniTypeSet types, LemniRatio64 val);
LemniTypedRatio128Expr lemniCreateTypedRatio128(LemniTypeSet types, LemniRatio128 val);

LemniTypedARatioExpr lemniTypedRatioAsARatio(LemniTypedRatioExpr q);
LemniTypedRatio32Expr lemniTypedRatioAsRatio32(LemniTypedRatioExpr q);
LemniTypedRatio64Expr lemniTypedRatioAsRatio64(LemniTypedRatioExpr q);
LemniTypedRatio128Expr lemniTypedRatioAsRatio128(LemniTypedRatioExpr q);

LemniARatioConst lemniTypedARatioValue(LemniTypedARatioExpr q);
LemniRatio32 lemniTypedRatio32Value(LemniTypedRatio32Expr q32);
LemniRatio64 lemniTypedRatio64Value(LemniTypedRatio64Expr q64);
LemniRatio128 lemniTypedRatio128Value(LemniTypedRatio128Expr q128);

LemniTypedRatioExpr lemniTypedARatioBase(LemniTypedARatioExpr q);
LemniTypedRatioExpr lemniTypedRatio32Base(LemniTypedRatio32Expr q32);
LemniTypedRatioExpr lemniTypedRatio64Base(LemniTypedRatio64Expr q64);
LemniTypedRatioExpr lemniTypedRatio128Base(LemniTypedRatio128Expr q128);

LemniTypedExpr lemniTypedARatioRoot(LemniTypedARatioExpr q);
LemniTypedExpr lemniTypedRatio32Root(LemniTypedRatio32Expr q32);
LemniTypedExpr lemniTypedRatio64Root(LemniTypedRatio64Expr q64);
LemniTypedExpr lemniTypedRatio128Root(LemniTypedRatio128Expr q128);

/***
 *  Real literal expressions
 ***/

LemniTypedRealExpr lemniTypedNumAsReal(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRealBase(LemniTypedRealExpr r);
LemniRealType lemniTypedRealType(LemniTypedRealExpr r);

LemniTypedARealExpr lemniCreateTypedAReal(LemniTypeSet types, LemniARealConst val);
LemniTypedReal32Expr lemniCreateTypedReal32(LemniTypeSet types, LemniReal32 val);
LemniTypedReal64Expr lemniCreateTypedReal64(LemniTypeSet types, LemniReal64 val);

LemniARealConst lemniTypedARealValue(LemniTypedARealExpr r);
LemniReal32 lemniTypedReal32Value(LemniTypedReal32Expr r32);
LemniReal64 lemniTypedReal64Value(LemniTypedReal64Expr r64);

LemniTypedRealExpr lemniTypedARealBase(LemniTypedARealExpr r);
LemniTypedRealExpr lemniTypedReal32Base(LemniTypedReal32Expr r32);
LemniTypedRealExpr lemniTypedReal64Base(LemniTypedReal64Expr r64);

LemniTypedExpr lemniTypedARealRoot(LemniTypedARealExpr r);
LemniTypedExpr lemniTypedReal32Root(LemniTypedReal32Expr real64);
LemniTypedExpr lemniTypedReal64Root(LemniTypedReal64Expr real64);

/***
 *  String literals expressions
 ***/

LemniTypedStringExpr lemniTypedConstantAsString(LemniTypedConstantExpr constant);
LemniTypedConstantExpr lemniTypedStringBase(LemniTypedStringExpr str);
LemniType lemniTypedStringType(LemniTypedStringExpr str);

/***
 *  ASCII String literal expressions
 ***/

LemniTypedStringASCIIExpr lemniCreateTypedStringASCII(LemniTypeSet types, LemniStrASCII val);
LemniTypedStringASCIIExpr lemniTypedStringAsASCII(LemniTypedStringExpr str);
LemniStrASCII lemniTypedStringASCIIValue(LemniTypedStringASCIIExpr ascii);
LemniStringASCIIType lemniTypedStringASCIIType(LemniTypedStringASCIIExpr ascii);
LemniTypedStringExpr lemniTypedStringASCIIBase(LemniTypedStringASCIIExpr ascii);
LemniTypedExpr lemniTypedStringASCIIRoot(LemniTypedStringASCIIExpr ascii);

/***
 *  UTF8 String literal expressions
 ***/

LemniTypedStringUTF8Expr lemniCreateTypedStringUTF8(LemniTypeSet types, LemniStrUTF8 val);
LemniTypedStringUTF8Expr lemniTypedStringAsUTF8(LemniTypedStringExpr str);
LemniStrUTF8 lemniTypedStringUTF8Value(LemniTypedStringUTF8Expr utf8);
LemniStringUTF8Type lemniTypedStringUTF8Type(LemniTypedStringUTF8Expr utf8);
LemniTypedStringExpr lemniTypedStringUTF8Base(LemniTypedStringUTF8Expr utf8);
LemniTypedExpr lemniTypedStringUTF8Root(LemniTypedStringUTF8Expr utf8);


LemniTypedLValueExpr lemniTypedExprAsLValue(LemniTypedExpr expr);
LemniTypedExpr lemniTypedLValueExprBase(LemniTypedLValueExpr lvalue);
LemniType lemniTypedLValueExprType(LemniTypedLValueExpr lvalue);
LemniStr lemniTypedLValueExprId(LemniTypedLValueExpr lvalue);

LemniTypedFnDefExpr lemniCreateTypedFnDef(LemniStr id, LemniTypedExpr *const params, const std::uint32_t numParams, LemniTypedExpr body);
LemniTypedFnDefExpr lemniTypedLValueExprAsFnDef(LemniTypedLValueExpr lvalue);
LemniTypedLValueExpr lemnitTypedFnDefExprBase(LemniTypedFnDefExpr fnDef);
LemniTypedFnExprType lemniTypedFnDefExprType(LemniTypedFnDefExpr fnDef);
LemniStr lemniTypedFnDefExprId(LemniTypedFnDefExpr fnDef);
uint32_t lemniTypedFnDefExprNumParams(LemniTypedFnDefExpr fnDef);
LemniTypedExpr lemniTypedFnDefExprParam(LemniTypedFnDefExpr fnDef, const uint32_t idx);
LemniTypedExpr lemniTypedFnDefExprBody(LemniTypedFnDefExpr fnDef);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
namespace lemni{
	using TypedExpr = LemniTypedExpr;

	class TypedConst{
		public:
			TypedConst() noexcept
				: TypedConst(nullptr){}

			TypedConst(TypedConst &&other) noexcept
				: m_val(other.m_val)
			{
				other.m_val = nullptr;
			}

			TypedConst(const TypedConst&) = delete;

			~TypedConst(){
				if(m_val) lemniDestroyTypedExpr(lemniTypedConstantRoot(m_val));
			}

			operator LemniTypedExpr() const noexcept{ return lemniTypedConstantRoot(m_val); }
			operator LemniTypedLiteralExpr() const noexcept{ return lemniTypedConstantBase(m_val); }
			operator LemniTypedConstantExpr() const noexcept{ return m_val; }

			TypedConst &operator=(TypedConst &&other) noexcept{
				if(m_val) lemniDestroyTypedExpr(lemniTypedConstantRoot(m_val));
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			TypedConst &operator=(const TypedConst&) = delete;

			static TypedConst from(LemniTypedConstantExpr const_) noexcept{
				return TypedConst(const_);
			}

			LemniType type() const noexcept{ return lemniTypedConstantType(m_val); }

		private:
			explicit TypedConst(LemniTypedConstantExpr const_) noexcept
				: m_val(const_){}

			LemniTypedConstantExpr m_val;
	};

	namespace detail{
		template<typename T, auto F> struct TypedValueCreatorMap{
			static auto create(LemniTypeSet types, T val) noexcept{ return TypedConst::from(F(types, val)); }
		};

		template<typename T> struct TypedValueCreator;

		template<> struct TypedValueCreator<LemniNat16>: TypedValueCreatorMap<LemniNat16, lemniCreateTypedNat16>{};
		template<> struct TypedValueCreator<LemniNat32>: TypedValueCreatorMap<LemniNat32, lemniCreateTypedNat32>{};
		template<> struct TypedValueCreator<LemniNat64>: TypedValueCreatorMap<LemniNat64, lemniCreateTypedNat64>{};

		template<> struct TypedValueCreator<LemniAIntConst>: TypedValueCreatorMap<LemniAIntConst, lemniCreateTypedAInt>{};
		template<> struct TypedValueCreator<LemniInt16>: TypedValueCreatorMap<LemniInt16, lemniCreateTypedInt16>{};
		template<> struct TypedValueCreator<LemniInt32>: TypedValueCreatorMap<LemniInt32, lemniCreateTypedInt32>{};
		template<> struct TypedValueCreator<LemniInt64>: TypedValueCreatorMap<LemniInt64, lemniCreateTypedInt64>{};

		template<> struct TypedValueCreator<LemniARatioConst>: TypedValueCreatorMap<LemniARatioConst, lemniCreateTypedARatio>{};
		template<> struct TypedValueCreator<LemniRatio32>: TypedValueCreatorMap<LemniRatio32, lemniCreateTypedRatio32>{};
		template<> struct TypedValueCreator<LemniRatio64>: TypedValueCreatorMap<LemniRatio64, lemniCreateTypedRatio64>{};
		template<> struct TypedValueCreator<LemniRatio128>: TypedValueCreatorMap<LemniRatio64, lemniCreateTypedRatio128>{};

		template<> struct TypedValueCreator<LemniARealConst>: TypedValueCreatorMap<LemniARealConst, lemniCreateTypedAReal>{};
		template<> struct TypedValueCreator<LemniReal32>: TypedValueCreatorMap<LemniReal32, lemniCreateTypedReal32>{};
		template<> struct TypedValueCreator<LemniReal64>: TypedValueCreatorMap<LemniReal64, lemniCreateTypedReal64>{};

		template<> struct TypedValueCreator<LemniStrASCII>: TypedValueCreatorMap<LemniStrASCII, lemniCreateTypedStringASCII>{};
		template<> struct TypedValueCreator<LemniStrUTF8>: TypedValueCreatorMap<LemniStrUTF8, lemniCreateTypedStringUTF8>{};
	}

	template<typename T>
	TypedConst lit(LemniTypeSet types, T t){ return detail::TypedValueCreator<T>::create(types, t); }
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_TYPEDEXPR_H
