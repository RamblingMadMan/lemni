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

#ifndef LEMNI_EXPR_H
#define LEMNI_EXPR_H 1

#include "Str.h"
#include "AReal.h"
#include "Operator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Exprs Expression related types and functions.
 * @{
 */

typedef const struct LemniExprT *LemniExpr;

typedef const struct LemniRefExprT *LemniRefExpr;

typedef const struct LemniApplicationExprT *LemniApplicationExpr;

typedef const struct LemniLiteralExprT *LemniLiteralExpr;
typedef const struct LemniConstantExprT *LemniConstantExpr;

typedef const struct LemniUnitExprT *LemniUnitExpr;

typedef const struct LemniNumExprT *LemniNumExpr;
typedef const struct LemniRealExprT *LemniRealExpr;
typedef const struct LemniRatioExprT *LemniRatioExpr;
typedef const struct LemniIntExprT *LemniIntExpr;

typedef const struct LemniStrExprT *LemniStrExpr;

typedef const struct LemniCommaListExprT *LemniCommaListExpr;
typedef const struct LemniUnaryOpExprT *LemniUnaryOpExpr;
typedef const struct LemniBinaryOpExprT *LemniBinaryOpExpr;

typedef const struct LemniFnDefExprT *LemniFnDefExpr;
typedef const struct LemniLambdaExprT *LemniLambdaExpr;

typedef const struct LemniBlockExprT *LemniBlockExpr;
typedef const struct LemniReturnExprT *LemniReturnExpr;

LemniStr lemniExprStr(LemniExpr expr);

LemniRefExpr lemniExprAsRef(LemniExpr expr);
LemniExpr lemniRefExprBase(LemniRefExpr ref);
LemniStr lemniRefExprId(LemniRefExpr ref);

LemniApplicationExpr lemniExprAsApplication(LemniExpr expr);
LemniExpr lemniApplicationExprBase(LemniApplicationExpr app);
LemniExpr lemniApplicationExprFn(LemniApplicationExpr app);
uint32_t lemniApplicationExprNumArgs(LemniApplicationExpr app);
LemniExpr lemniApplicationExprArg(LemniApplicationExpr app, const uint32_t idx);

LemniLiteralExpr lemniExprAsLiteral(LemniExpr expr);
LemniExpr lemniLiteralExprBase(LemniLiteralExpr lit);

LemniConstantExpr lemniLiteralExprAsConstant(LemniLiteralExpr lit);
LemniLiteralExpr lemniConstantExprBase(LemniConstantExpr constant);

LemniUnitExpr LemniConstantExprAsUnit(LemniConstantExpr constant);
LemniExpr lemniUnitExprBase(LemniUnitExpr unit);

LemniNumExpr lemniConstantExprAsNum(LemniConstantExpr constant);
LemniConstantExpr lemniNumExprBase(LemniNumExpr num);

LemniRealExpr lemniNumExprAsReal(LemniNumExpr num);
LemniNumExpr lemniRealExprBase(LemniRealExpr real);
LemniARealConst lemniRealExprValue(LemniRealExpr real);

LemniRatioExpr lemniNumExprAsRatio(LemniNumExpr num);
LemniNumExpr lemniRatioExprBase(LemniRatioExpr ratio);
LemniARatioConst lemniRatioExprValue(LemniRatioExpr ratio);

LemniIntExpr lemniNumExprAsInt(LemniNumExpr num);
LemniNumExpr lemniIntExprBase(LemniIntExpr int_);
LemniAIntConst lemniIntExprValue(LemniIntExpr int_);

LemniStrExpr lemniConstantExprAsStr(LemniConstantExpr constant);
LemniConstantExpr lemniStrExprBase(LemniStrExpr str);
LemniStr lemniStrExprValue(LemniStrExpr str);

LemniCommaListExpr lemniExprAsCommaList(LemniExpr expr);
LemniExpr lemniCommaListExprBase(LemniCommaListExpr list);
uint32_t lemniCommaListExprNumElements(LemniCommaListExpr list);
LemniExpr lemniCommaListExprElement(LemniCommaListExpr list, const uint32_t idx);

LemniUnaryOpExpr lemniExprAsUnaryOp(LemniExpr expr);
LemniExpr lemniUnaryOpExprBase(LemniUnaryOpExpr unaryOp);
LemniUnaryOp lemniUnaryOpExprOp(LemniUnaryOpExpr unaryOp);
LemniExpr lemniUnaryOpExprValue(LemniUnaryOpExpr unaryOp);

LemniBinaryOpExpr lemniExprAsBinaryOp(LemniExpr expr);
LemniExpr lemniBinaryOpExprBase(LemniBinaryOpExpr binaryOp);
LemniBinaryOp lemniBinaryOpExprOp(LemniBinaryOpExpr binaryOp);
LemniExpr lemniBinaryOpExprLhs(LemniBinaryOpExpr binaryOp);
LemniExpr lemniBinaryOpExprRhs(LemniBinaryOpExpr binaryOp);

LemniFnDefExpr lemniExprAsFnDef(LemniExpr expr);
LemniExpr lemniFnDefExprBase(LemniFnDefExpr fnDef);
LemniStr lemniFnDefExprName(LemniFnDefExpr fnDef);
uint32_t lemniFnDefExprNumParams(LemniFnDefExpr fnDef);
LemniExpr lemniFnDefExprParam(LemniFnDefExpr fnDef, const uint32_t idx);
LemniExpr lemniFnDefExprBody(LemniFnDefExpr fnDef);

LemniLambdaExpr lemniExprAsLambda(LemniExpr expr);
LemniExpr lemniLambdaExprBase(LemniLambdaExpr lambda);
uint32_t lemniLambdaExprNumParams(LemniLambdaExpr lambda);
LemniExpr lemniLambdaExprParam(LemniLambdaExpr lambda, const uint32_t idx);
LemniExpr lemniLambdaExprBody(LemniLambdaExpr lambda);

LemniBlockExpr lemniExprAsBlock(LemniExpr expr);
LemniExpr lemniBlockExprBase(LemniBlockExpr block);
uint32_t lemniBlockExprNumExprs(LemniBlockExpr block);
LemniExpr lemniBlockExprExpr(LemniBlockExpr block, const uint32_t idx);

LemniReturnExpr lemniExprAsReturn(LemniExpr expr);
LemniExpr lemniReturnExprBase(LemniReturnExpr return_);
LemniExpr lemniReturnExprValue(LemniReturnExpr return_);

/**
 * @}
 */

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#define LEMNI_ALIAS_FN(name, alias) inline constexpr auto &&alias = name

namespace lemni{
	LEMNI_ALIAS_FN(lemniExprAsRef, exprAsRef);
	LEMNI_ALIAS_FN(lemniRefExprBase, refExprBase);
	LEMNI_ALIAS_FN(lemniRefExprId, refExprId);

	LEMNI_ALIAS_FN(lemniExprAsLiteral, exprAsLiteral);
	LEMNI_ALIAS_FN(lemniLiteralExprBase, literalExprBase);

	LEMNI_ALIAS_FN(lemniLiteralExprAsConstant, literalExprAsConstant);
	LEMNI_ALIAS_FN(lemniConstantExprBase, constantExprBase);

	LEMNI_ALIAS_FN(lemniConstantExprAsNum, constantExprAsNum);
	LEMNI_ALIAS_FN(lemniNumExprBase, numExprBase);

	LEMNI_ALIAS_FN(lemniNumExprAsReal, numExprAsReal);
	LEMNI_ALIAS_FN(lemniRealExprBase, realExprBase);
	LEMNI_ALIAS_FN(lemniRealExprValue, realExprValue);

	LEMNI_ALIAS_FN(lemniNumExprAsRatio, numExprAsRatio);
	LEMNI_ALIAS_FN(lemniRatioExprBase, ratioExprBase);
	LEMNI_ALIAS_FN(lemniRatioExprValue, ratioExprValue);

	LEMNI_ALIAS_FN(lemniNumExprAsInt, numExprAsInt);
	LEMNI_ALIAS_FN(lemniIntExprBase, intExprBase);
	LEMNI_ALIAS_FN(lemniIntExprValue, intExprValue);

	LEMNI_ALIAS_FN(lemniConstantExprAsStr, constantExprAsStr);
	LEMNI_ALIAS_FN(lemniStrExprBase, strExprBase);
	LEMNI_ALIAS_FN(lemniStrExprValue, strExprValue);

	LEMNI_ALIAS_FN(lemniExprAsCommaList, exprAsCommaList);
	LEMNI_ALIAS_FN(lemniCommaListExprBase, commaListExprBase);
	LEMNI_ALIAS_FN(lemniCommaListExprNumElements, commaListExprNumElements);
	LEMNI_ALIAS_FN(lemniCommaListExprElement, commaListExprElement);

	LEMNI_ALIAS_FN(lemniExprAsUnaryOp, exprAsUnaryOp);
	LEMNI_ALIAS_FN(lemniUnaryOpExprBase, unaryOpExprBase);
	LEMNI_ALIAS_FN(lemniUnaryOpExprOp, unaryOpExprOp);
	LEMNI_ALIAS_FN(lemniUnaryOpExprValue, unaryOpExprValue);

	LEMNI_ALIAS_FN(lemniExprAsBinaryOp, exprAsBinaryOp);
	LEMNI_ALIAS_FN(lemniBinaryOpExprBase, binaryOpExprBase);
	LEMNI_ALIAS_FN(lemniBinaryOpExprOp, binaryOpExprOp);
	LEMNI_ALIAS_FN(lemniBinaryOpExprLhs, binaryOpExprLhs);
	LEMNI_ALIAS_FN(lemniBinaryOpExprRhs, binaryOpExprRhs);

	LEMNI_ALIAS_FN(lemniExprAsFnDef, exprAsFnDef);
	LEMNI_ALIAS_FN(lemniFnDefExprBase, fnDefExprBase);
	LEMNI_ALIAS_FN(lemniFnDefExprName, fnDefExprName);
	LEMNI_ALIAS_FN(lemniFnDefExprNumParams, fnDefExprNumParams);
	LEMNI_ALIAS_FN(lemniFnDefExprParam, fnDefExprParam);
	LEMNI_ALIAS_FN(lemniFnDefExprBody, fnDefExprBody);

	LEMNI_ALIAS_FN(lemniExprAsLambda, exprAsLambda);
	LEMNI_ALIAS_FN(lemniLambdaExprBase, lambdaExprBase);
	LEMNI_ALIAS_FN(lemniLambdaExprNumParams, lambdaExprNumParams);
	LEMNI_ALIAS_FN(lemniLambdaExprParam, lambdaExprParam);
	LEMNI_ALIAS_FN(lemniLambdaExprBody, lambdaExprBody);

	LEMNI_ALIAS_FN(lemniExprAsBlock, exprAsBlock);
	LEMNI_ALIAS_FN(lemniBlockExprBase, blockExprBase);
	LEMNI_ALIAS_FN(lemniBlockExprNumExprs, blockExprNumExprs);
	LEMNI_ALIAS_FN(lemniBlockExprExpr, blockExprExpr);

	LEMNI_ALIAS_FN(lemniExprAsReturn, exprAsReturn);
	LEMNI_ALIAS_FN(lemniReturnExprBase, returnExprBase);
	LEMNI_ALIAS_FN(lemniReturnExprValue, returnExprValue);
}

#undef LEMNI_ALIAS_FN
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_EXPR_H
