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

typedef const struct LemniExprT *LemniExpr;

typedef const struct LemniLiteralExprT *LemniLiteralExpr;
typedef const struct LemniConstantExprT *LemniConstantExpr;

typedef const struct LemniNumExprT *LemniNumExpr;
typedef const struct LemniRealExprT *LemniRealExpr;
typedef const struct LemniRatioExprT *LemniRatioExpr;
typedef const struct LemniIntExprT *LemniIntExpr;

typedef const struct LemniStrExprT *LemniStrExpr;

typedef const struct LemniUnaryOpExprT *LemniUnaryOpExpr;
typedef const struct LemniBinaryOpExprT *LemniBinaryOpExpr;

typedef const struct LemniFnDefExprT *LemniFnDefExpr;
typedef const struct LemniLambdaExprT *LemniLambdaExpr;

typedef const struct LemniBlockExprT *LemniBlockExpr;
typedef const struct LemniReturnExprT *LemniReturnExpr;

LemniStr lemniExprStr(LemniExpr expr);

LemniLiteralExpr lemniExprAsLiteral(LemniExpr expr);
LemniExpr lemniLiteralExprBase(LemniLiteralExpr lit);

LemniConstantExpr lemniLiteralExprAsConstant(LemniLiteralExpr lit);
LemniLiteralExpr lemniConstantExprBase(LemniConstantExpr constant);

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

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_EXPR_H
