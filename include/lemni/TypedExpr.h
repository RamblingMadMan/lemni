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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup TypedExprs Typed expression related types and functions.
 * @{
 */

LEMNI_OPAQUE_CONST_T(LemniTypedExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedUnaryOpExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedBinaryOpExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedApplicationExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedLiteralExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedProductExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedLambdaExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedConstantExpr);

LEMNI_OPAQUE_CONST_T(LemniTypedImportExpr);
LEMNI_OPAQUE_CONST_T(LemniTypedExportExpr);

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


LemniTypedConstantExpr lemniTypedLiteralExprAsConstant(LemniTypedLiteralExpr expr);
LemniTypedLiteralExpr lemniTypedConstantExprBase(LemniTypedConstantExpr constant);
LemniType lemniTypedConstantExprType(LemniTypedConstantExpr constant);

LemniTypedNumExpr lemniTypedConstantExprAsNum(LemniTypedConstantExpr constant);
LemniTypedConstantExpr lemniTypedNumExprBase(LemniTypedNumExpr num);
LemniType lemniTypedNumExprType(LemniTypedNumExpr num);

LemniTypedNatExpr lemniCreateTypedNat(LemniAIntConst val);
LemniTypedNatExpr lemniTypedNumExprAsNat(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedNatExprBase(LemniTypedNatExpr nat);
LemniNatType lemniTypedNatExprType(LemniTypedNatExpr nat);
LemniAIntConst lemniTypedNatExprValue(LemniTypedNatExpr nat);

LemniTypedIntExpr lemniCreateTypedInt(LemniAIntConst val);
LemniTypedIntExpr lemniTypedNumExprAsInt(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedIntExprBase(LemniTypedIntExpr int_);
LemniIntType lemniTypedIntExprType(LemniTypedIntExpr int_);
LemniAIntConst lemniTypedIntExprValue(LemniTypedIntExpr int_);

LemniTypedRatioExpr lemniCreateTypedRatio(LemniARatioConst val);
LemniTypedRatioExpr lemniTypedNumExprAsRatio(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRatioExprBase(LemniTypedRatioExpr ratio);
LemniRatioType lemniTypedRatioExprType(LemniTypedRatioExpr ratio);
LemniARatioConst lemniTypedRatioExprValue(LemniTypedRatioExpr ratio);

LemniTypedRealExpr lemniCreateTypedReal(LemniARealConst val);
LemniTypedRealExpr lemniTypedNumExprAsReal(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRealExprBase(LemniTypedRealExpr real);
LemniRealType lemniTypedRealExprType(LemniTypedRealExpr real);
LemniARealConst lemniTypedRealExprValue(LemniTypedRealExpr real);

LemniTypedStringExpr lemniTypedConstantExprAsString(LemniTypedConstantExpr constant);
LemniTypedConstantExpr lemniTypedStringExprBase(LemniTypedStringExpr str);
LemniType lemniTypedStringExprType(LemniTypedStringExpr str);

LemniTypedStringASCIIExpr lemniCreateTypedStringASCII(LemniStr val);
LemniTypedStringASCIIExpr lemniTypedStringExprAsASCII(LemniTypedStringExpr str);
LemniTypedStringExpr lemniTypedStringASCIIExprBase(LemniTypedStringASCIIExpr ascii);
LemniStringASCIIType lemniTypedStringASCIIExprType(LemniTypedStringASCIIExpr ascii);
LemniStr lemniTypedStringASCIIExprValue(LemniTypedStringASCIIExpr ascii);

LemniTypedStringUTF8Expr lemniCreateTypedStringUTF8(LemniStr val);
LemniTypedStringUTF8Expr lemniTypedStringExprAsUTF8(LemniTypedStringExpr str);
LemniTypedStringExpr lemniTypedStringUTF8ExprBase(LemniTypedStringUTF8Expr utf8);
LemniStringUTF8Type lemniTypedStringUTF8ExprType(LemniTypedStringUTF8Expr utf8);
LemniStr lemniTypedStringUTF8ExprValue(LemniTypedStringUTF8Expr utf8);

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

/**
 * @}
 */

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
namespace lemni{
	using TypedExpr = LemniTypedExpr;
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_TYPEDEXPR_H
