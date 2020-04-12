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

typedef const struct LemniTypedExprT *LemniTypedExpr;

typedef const struct LemniTypedUnaryOpExprT *LemniTypedUnaryOpExpr;
typedef const struct LemniTypedBinaryOpExprT *LemniTypedBinaryOpExpr;

typedef const struct LemniTypeApplicationExprT *LemniTypedApplicationExpr;


typedef const struct LemniTypedLiteralExprT *LemniTypedLiteralExpr;

typedef const struct LemniTypedLambdaExprT *LemniTypedLambdaExpr;


typedef const struct LemniTypedConstantExprT *LemniTypedConstantExpr;

typedef const struct LemniTypedNumExprT *LemniTypedNumExpr;
typedef const struct LemniTypedNatExprT *LemniTypedNatExpr;
typedef const struct LemniTypedIntExprT *LemniTypedIntExpr;
typedef const struct LemniTypedRatioExprT *LemniTypedRatioExpr;
typedef const struct LemniTypedRealExprT *LemniTypedRealExpr;

typedef const struct LemniTypedStringExprT *LemniTypedStringExpr;
typedef const struct LemniTypedStringASCIIExprT *LemniTypedStringASCIIExpr;
typedef const struct LemniTypedStringUTF8ExprT *LemniTypedStringUTF8Expr;


typedef const struct LemniTypedLValueExprT *LemniTypedLValueExpr;

typedef const struct LemniTypedFnDefExprT *LemniTypedFnDefExpr;


typedef struct {
	bool isClosure;
	union {
		LemniFunctionType function;
		LemniClosureType closure;
	};
} LemniTypedFnExprType;

LemniType lemniTypedExprType(LemniTypedExpr expr);

LemniTypedUnaryOpExpr lemniTypedExprAsUnaryOp(LemniTypedExpr expr);
LemniTypedExpr lemniTypedUnaryOpExprBase(LemniTypedUnaryOpExpr unaryOp);
LemniType lemniTypedUnaryOpExprType(LemniTypedUnaryOpExpr unaryOp);
LemniTypedExpr lemniTypedUnaryOpExprValue(LemniTypedUnaryOpExpr unaryOp);
LemniUnaryOp lemniTypedUnaryOpExprOp(LemniTypedUnaryOpExpr unaryOp);

LemniTypedBinaryOpExpr lemniTypedExprAsBinaryOp(LemniTypedExpr expr);
LemniTypedExpr lemniTypedBinaryOpExprBase(LemniTypedBinaryOpExpr binaryOp);
LemniType lemniTypedBinaryOpExprType(LemniTypedBinaryOpExpr binaryOp);
LemniTypedExpr lemniTypedBinaryOpExprLhs(LemniTypedBinaryOpExpr binaryOp);
LemniTypedExpr lemniTypedBinaryOpExprRhs(LemniTypedBinaryOpExpr binaryOp);
LemniBinaryOp lemniTypedBinaryOpExprOp(LemniTypedBinaryOpExpr binaryOp);

LemniTypedApplicationExpr lemniTypedExprAsApplication(LemniTypedExpr expr);
LemniTypedExpr lemniTypedApplicationExprBase(LemniTypedApplicationExpr application);
LemniType lemniTypedApplicationExprType(LemniTypedApplicationExpr application);
LemniTypedExpr lemniTypedApplicationExprFunctor(LemniTypedApplicationExpr application);
uint32_t lemniTypedApplicationExprNumArgs(LemniTypedApplicationExpr application);
LemniTypedExpr lemniTypedApplicationExprArg(LemniTypedApplicationExpr application, const uint32_t idx);

LemniTypedLiteralExpr lemniTypedExprAsLiteral(LemniTypedExpr expr);
LemniTypedExpr lemniTypedLiteralExprBase(LemniTypedLiteralExpr lit);
LemniType lemniTypedLiteralExprType(LemniTypedLiteralExpr lit);

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

LemniTypedNatExpr lemniTypedNumExprAsNat(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedNatExprBase(LemniTypedNatExpr nat);
LemniNatType lemniTypedNatExprType(LemniTypedNatExpr nat);
LemniAIntConst lemniTypedNatExprValue(LemniTypedNatExpr nat);

LemniTypedIntExpr lemniTypedNumExprAsInt(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedIntExprBase(LemniTypedIntExpr int_);
LemniIntType lemniTypedIntExprType(LemniTypedIntExpr int_);
LemniAIntConst lemniTypedIntExprValue(LemniTypedIntExpr int_);

LemniTypedRatioExpr lemniTypedNumExprAsRatio(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRatioExprBase(LemniTypedRatioExpr ratio);
LemniRatioType lemniTypedRatioExprType(LemniTypedRatioExpr ratio);
LemniARatioConst lemniTypedRatioExprValue(LemniTypedRatioExpr ratio);

LemniTypedRealExpr lemniTypedNumExprAsReal(LemniTypedNumExpr num);
LemniTypedNumExpr lemniTypedRealExprBase(LemniTypedRealExpr real);
LemniRealType lemniTypedRealExprType(LemniTypedRealExpr real);
LemniARealConst lemniTypedRealExprValue(LemniTypedRealExpr real);

LemniTypedStringExpr lemniTypedConstantExprAsString(LemniTypedConstantExpr constant);
LemniTypedConstantExpr lemniTypedStringExprBase(LemniTypedStringExpr str);
LemniType lemniTypedStringExprType(LemniTypedStringExpr str);

LemniTypedStringASCIIExpr lemniTypedStringExprAsASCII(LemniTypedStringExpr str);
LemniTypedStringExpr lemniTypedStringASCIIExprBase(LemniTypedStringASCIIExpr ascii);
LemniStringASCIIType lemniTypedStringASCIIExprType(LemniTypedStringASCIIExpr ascii);
LemniStr lemniTypedStringASCIIExprValue(LemniTypedStringASCIIExpr ascii);

LemniTypedStringUTF8Expr lemniTypedStringExprAsUTF8(LemniTypedStringExpr str);
LemniTypedStringExpr lemniTypedStringUTF8ExprBase(LemniTypedStringUTF8Expr utf8);
LemniStringUTF8Type lemniTypedStringUTF8ExprType(LemniTypedStringUTF8Expr utf8);
LemniStr lemniTypedStringUTF8ExprValue(LemniTypedStringUTF8Expr utf8);

LemniTypedLValueExpr lemniTypedExprAsLValue(LemniTypedExpr expr);
LemniTypedExpr lemniTypedLValueExprBase(LemniTypedLValueExpr lvalue);
LemniType lemniTypedLValueExprType(LemniTypedLValueExpr lvalue);

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
