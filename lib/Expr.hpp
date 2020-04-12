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

#ifndef LEMNI_LIB_EXPR_HPP
#define LEMNI_LIB_EXPR_HPP 1

#include <vector>
#include <string>

#include "lemni/Expr.h"

struct LemniExprT{
	explicit LemniExprT(LemniLocation loc_): loc(loc_){}
	virtual ~LemniExprT() = default;

	LemniLocation loc;
};

struct LemniApplicationExprT: LemniExprT{
	LemniApplicationExprT(LemniLocation loc_, LemniExpr fn_, std::vector<LemniExpr> args_)
		: LemniExprT(loc_), fn(fn_), args(std::move(args_)){}

	LemniExpr fn;
	std::vector<LemniExpr> args;
};

struct LemniLiteralExprT: LemniExprT{ using LemniExprT::LemniExprT; };

struct LemniTupleExprT: LemniLiteralExprT{
	LemniTupleExprT(LemniLocation loc_, std::vector<LemniExpr> elements_)
		: LemniLiteralExprT(loc_), elements(std::move(elements_)){}

	std::vector<LemniExpr> elements;
};

struct LemniConstantExprT: LemniLiteralExprT{ using LemniLiteralExprT::LemniLiteralExprT; };

struct LemniUnitExprT: LemniConstantExprT{ using LemniConstantExprT::LemniConstantExprT; };

struct LemniNumExprT: LemniConstantExprT{ using LemniConstantExprT::LemniConstantExprT; };

struct LemniRealExprT: LemniNumExprT{
	LemniRealExprT(LemniLocation loc_, LemniStr str, const int base)
		: LemniNumExprT(loc_), val(lemniCreateARealStr(str, base)){}

	~LemniRealExprT() override{ lemniDestroyAReal(val); }

	LemniAReal val;
};

struct LemniRatioExprT: LemniNumExprT{
	LemniRatioExprT(LemniLocation loc_, LemniStr str, const int base)
		: LemniNumExprT(loc_), val(lemniCreateARatioStr(str, base)){}

	~LemniRatioExprT() override{ lemniDestroyARatio(val); }

	LemniARatio val;
};

struct LemniIntExprT: LemniNumExprT{
	LemniIntExprT(LemniLocation loc_, LemniStr str, const int base)
		: LemniNumExprT(loc_), val(lemniCreateAIntStr(str, base)){}

	~LemniIntExprT() override{ lemniDestroyAInt(val); }

	LemniAInt val;
};

struct LemniStrExprT: LemniConstantExprT{
	explicit LemniStrExprT(LemniLocation loc_, std::string str)
		: LemniConstantExprT(loc_), val(std::move(str)){}

	std::string val;
};

struct LemniCommaListExprT: LemniExprT{
	explicit LemniCommaListExprT(LemniLocation loc_, std::vector<LemniExpr> elements_)
		: LemniExprT(loc_), elements(std::move(elements_)){}

	std::vector<LemniExpr> elements;
};

struct LemniUnaryOpExprT: LemniExprT{
	LemniUnaryOpExprT(LemniLocation loc_, LemniUnaryOp op_, LemniExpr expr_)
		: LemniExprT(loc_), op(op_), expr(expr_){}

	LemniUnaryOp op;
	LemniExpr expr;
};

struct LemniBinaryOpExprT: LemniExprT{
	LemniBinaryOpExprT(LemniLocation loc_, LemniBinaryOp op_, LemniExpr lhs_, LemniExpr rhs_)
		: LemniExprT(loc_), op(op_), lhs(lhs_), rhs(rhs_){}

	LemniBinaryOp op;
	LemniExpr lhs, rhs;
};

struct LemniLambdaExprT: LemniExprT{
	LemniLambdaExprT(LemniLocation loc_, std::vector<LemniExpr> params_, LemniExpr body_)
		: LemniExprT(loc_), params(std::move(params_)), body(body_){}

	std::vector<LemniExpr> params;
	LemniExpr body;
};

struct LemniBlockExprT: LemniExprT{
	explicit LemniBlockExprT(LemniLocation loc_, std::vector<LemniExpr> exprs_)
		: LemniExprT(loc_), exprs(std::move(exprs_)){}

	std::vector<LemniExpr> exprs;
};

struct LemniReturnExprT: LemniExprT{
	explicit LemniReturnExprT(LemniLocation loc_, LemniExpr expr_)
		: LemniExprT(loc_), expr(expr_){}

	LemniExpr expr;
};

struct LemniLValueExprT: LemniExprT{ using LemniExprT::LemniExprT; };

struct LemniRefExprT: LemniLValueExprT{
	LemniRefExprT(LemniLocation loc_, std::string id_)
		: LemniLValueExprT(loc_), id(std::move(id_)){}

	std::string id;
};

struct LemniFnDefExprT: LemniLValueExprT{
	LemniFnDefExprT(LemniLocation loc_, std::string name_, std::vector<LemniExpr> params_, LemniExpr body_)
		: LemniLValueExprT(loc_), name(std::move(name_)), params(std::move(params_)), body(body_){}

	std::string name;
	std::vector<LemniExpr> params;
	LemniExpr body;
};

#endif // !LEMNI_LIB_EXPR_HPP
