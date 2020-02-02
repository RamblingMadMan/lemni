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

#define LEMNI_NO_CPP
#include "Expr.hpp"

LemniRefExpr lemniExprAsRef(LemniExpr expr){ return dynamic_cast<LemniRefExpr>(expr); }
LemniExpr lemniRefExprBase(LemniRefExpr ref){ return ref; }
LemniStr lemniRefExprId(LemniRefExpr ref){ return LemniStr{ref->id.c_str(), ref->id.size()}; }

LemniApplicationExpr lemniExprAsApplication(LemniExpr expr){ return dynamic_cast<LemniApplicationExpr>(expr); }
LemniExpr lemniApplicationExprBase(LemniApplicationExpr app){ return app; }
LemniExpr lemniApplicationExprFn(LemniApplicationExpr app){ return app->fn; }
uint32_t LemniApplicationExprNumArgs(LemniApplicationExpr app){ return static_cast<uint32_t>(app->args.size()); }
LemniExpr LemniApplicationExprArg(LemniApplicationExpr app, const uint32_t idx){ return app->args[idx]; }

LemniLiteralExpr lemniExprAsLiteral(LemniExpr expr){ return dynamic_cast<LemniLiteralExpr>(expr); }
LemniExpr lemniLiteralExprBase(LemniLiteralExpr lit){ return lit; }

LemniConstantExpr lemniLiteralExprAsConstant(LemniLiteralExpr lit){ return dynamic_cast<LemniConstantExpr>(lit); }
LemniLiteralExpr lemniConstantExprBase(LemniConstantExpr constant){ return constant; }

LemniNumExpr lemniConstantExprAsNum(LemniConstantExpr constant){ return dynamic_cast<LemniNumExpr>(constant); }
LemniConstantExpr lemniNumExprBase(LemniNumExpr num){ return num; }

LemniRealExpr lemniNumExprAsReal(LemniNumExpr num){ return dynamic_cast<LemniRealExpr>(num); }
LemniNumExpr lemniRealExprBase(LemniRealExpr real){ return real; }
LemniARealConst lemniRealExprValue(LemniRealExpr real){ return real->val; }

LemniRatioExpr lemniNumExprAsRatio(LemniNumExpr num){ return dynamic_cast<LemniRatioExpr>(num); }
LemniNumExpr lemniRatioExprBase(LemniRatioExpr ratio){ return ratio; }
LemniARatioConst lemniRatioExprValue(LemniRatioExpr ratio){ return ratio->val; }

LemniIntExpr lemniNumExprAsInt(LemniNumExpr num){ return dynamic_cast<LemniIntExpr>(num); }
LemniNumExpr lemniIntExprBase(LemniIntExpr int_){ return int_; }
LemniAIntConst lemniIntExprValue(LemniIntExpr int_){ return int_->val; }

LemniStrExpr lemniConstantExprAsStr(LemniConstantExpr constant){ return dynamic_cast<LemniStrExpr>(constant); }
LemniConstantExpr lemniStrExprBase(LemniStrExpr str){ return str; }
LemniStr lemniStrExprValue(LemniStrExpr str){ return LemniStr{str->val.c_str(), str->val.size()}; }

LemniUnaryOpExpr lemniExprAsUnaryOp(LemniExpr expr){ return dynamic_cast<LemniUnaryOpExpr>(expr); }
LemniExpr lemniUnaryOpExprBase(LemniUnaryOpExpr unaryOp){ return unaryOp; }
LemniUnaryOp lemniUnaryOpExprOp(LemniUnaryOpExpr unaryOp){ return unaryOp->op; }
LemniExpr lemniUnaryOpExprValue(LemniUnaryOpExpr unaryOp){ return unaryOp->expr; }

LemniBinaryOpExpr lemniExprAsBinaryOp(LemniExpr expr){ return dynamic_cast<LemniBinaryOpExpr>(expr); }
LemniExpr lemniBinaryOpExprBase(LemniBinaryOpExpr binaryOp){ return binaryOp; }
LemniBinaryOp lemniBinaryOpExprOp(LemniBinaryOpExpr binaryOp){ return binaryOp->op; }
LemniExpr lemniBinaryOpExprLhs(LemniBinaryOpExpr binaryOp){ return binaryOp->lhs; }
LemniExpr lemniBinaryOpExprRhs(LemniBinaryOpExpr binaryOp){ return binaryOp->rhs; }

LemniFnDefExpr lemniExprAsFnDef(LemniExpr expr){ return dynamic_cast<LemniFnDefExpr>(expr); }
LemniExpr lemniFnDefExprBase(LemniFnDefExpr fnDef){ return fnDef; }
LemniStr lemniFnDefExprName(LemniFnDefExpr fnDef){ return LemniStr{fnDef->name.c_str(), fnDef->name.size()}; }
uint32_t lemniFnDefExprNumParams(LemniFnDefExpr fnDef){ return static_cast<uint32_t>(fnDef->params.size()); }
LemniExpr lemniFnDefExprParam(LemniFnDefExpr fnDef, const uint32_t idx){ return fnDef->params[idx]; }
LemniExpr lemniFnDefExprBody(LemniFnDefExpr fnDef){ return fnDef->body; }

LemniLambdaExpr lemniExprAsLambda(LemniExpr expr){ return dynamic_cast<LemniLambdaExpr>(expr); }
LemniExpr lemniLambdaExprBase(LemniLambdaExpr lambda){ return lambda; }
uint32_t lemniLambdaExprNumParams(LemniLambdaExpr lambda){ return static_cast<uint32_t>(lambda->params.size()); }
LemniExpr lemniLambdaExprParam(LemniLambdaExpr lambda, const uint32_t idx){ return lambda->params[idx]; }
LemniExpr lemniLambdaExprBody(LemniLambdaExpr lambda){ return lambda->body; }

LemniBlockExpr lemniExprAsBlock(LemniExpr expr){ return dynamic_cast<LemniBlockExpr>(expr); }
LemniExpr lemniBlockExprBase(LemniBlockExpr block){ return block; }
uint32_t lemniBlockExprNumExprs(LemniBlockExpr block){ return static_cast<uint32_t>(block->exprs.size()); }
LemniExpr lemniBlockExprExpr(LemniBlockExpr block, const uint32_t idx){ return block->exprs[idx]; }

LemniReturnExpr lemniExprAsReturn(LemniExpr expr){ return dynamic_cast<LemniReturnExpr>(expr); }
LemniExpr lemniReturnExprBase(LemniReturnExpr return_){ return return_; }
LemniExpr lemniReturnExprValue(LemniReturnExpr return_){ return return_->expr; }
