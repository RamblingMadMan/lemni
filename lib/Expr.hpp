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
	virtual ~LemniExprT() = default;
	//virtual LemniStr toStr() const noexcept = 0;
};

struct LemniRefExprT: LemniExprT{
	explicit LemniRefExprT(std::string id_)
		: id(std::move(id_)){}

	std::string id;
};

struct LemniApplicationExprT: LemniExprT{
	LemniApplicationExprT(LemniExpr fn_, std::vector<LemniExpr> args_)
		: fn(fn_), args(std::move(args_)){}

	LemniExpr fn;
	std::vector<LemniExpr> args;
};

struct LemniLiteralExprT: LemniExprT{};
struct LemniConstantExprT: LemniLiteralExprT{};

struct LemniUnitExprT: LemniConstantExprT{};

struct LemniNumExprT: LemniConstantExprT{};

struct LemniRealExprT: LemniNumExprT{
	LemniRealExprT(LemniStr str, const int base)
		: val(lemniCreateARealStr(str, base)){}

	~LemniRealExprT(){ lemniDestroyAReal(val); }

	LemniAReal val;
};

struct LemniRatioExprT: LemniNumExprT{
	LemniRatioExprT(LemniStr str, const int base)
		: val(lemniCreateARatioStr(str, base)){}

	~LemniRatioExprT(){ lemniDestroyARatio(val); }

	LemniARatio val;
};

struct LemniIntExprT: LemniNumExprT{
	LemniIntExprT(LemniStr str, const int base)
		: val(lemniCreateAIntStr(str, base)){}

	~LemniIntExprT(){ lemniDestroyAInt(val); }

	LemniAInt val;
};

struct LemniStrExprT: LemniConstantExprT{
	explicit LemniStrExprT(std::string str)
		: val(std::move(str)){}

	std::string val;
};

struct LemniUnaryOpExprT: LemniExprT{
	LemniUnaryOpExprT(LemniUnaryOp op_, LemniExpr expr_)
		: op(op_), expr(expr_){}

	LemniUnaryOp op;
	LemniExpr expr;
};

struct LemniBinaryOpExprT: LemniExprT{
	LemniBinaryOpExprT(LemniBinaryOp op_, LemniExpr lhs_, LemniExpr rhs_)
		: op(op_), lhs(lhs_), rhs(rhs_){}

	LemniBinaryOp op;
	LemniExpr lhs, rhs;
};

struct LemniFnDefExprT: LemniExprT{
	LemniFnDefExprT(std::string name_, std::vector<LemniExpr> params_, LemniExpr body_)
		: name(std::move(name_)), params(std::move(params_)), body(body_){}

	std::string name;
	std::vector<LemniExpr> params;
	LemniExpr body;
};

struct LemniLambdaExprT: LemniExprT{
	LemniLambdaExprT(std::vector<LemniExpr> params_, LemniExpr body_)
		: params(std::move(params_)), body(body_){}

	std::vector<LemniExpr> params;
	LemniExpr body;
};

struct LemniBlockExprT: LemniExprT{
	explicit LemniBlockExprT(std::vector<LemniExpr> exprs_)
			: exprs(std::move(exprs_)){}

	std::vector<LemniExpr> exprs;
};

struct LemniReturnExprT: LemniExprT{
	explicit LemniReturnExprT(LemniExpr expr_)
		: expr(expr_){}

	LemniExpr expr;
};

#endif // !LEMNI_LIB_EXPR_HPP
