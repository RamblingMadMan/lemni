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

#include "utf8.h"

#include "lemni/AReal.h"
#include "lemni/Expr.h"
#include "lemni/Type.h"
#include "lemni/Scope.h"
#include "lemni/typecheck.h"

#include "TypedExpr.hpp"
#include "Type.hpp"

namespace {
	inline unsigned long ceilPowerOfTwo(unsigned long v){
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
}

struct LemniExprT{
	explicit LemniExprT(LemniLocation loc_): loc(loc_){}
	virtual ~LemniExprT() = default;

	virtual LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept = 0;

	LemniLocation loc;
};

struct LemniApplicationExprT: LemniExprT{
	LemniApplicationExprT(LemniLocation loc_, LemniExpr fn_, std::vector<LemniExpr> args_)
		: LemniExprT(loc_), fn(fn_), args(std::move(args_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniExpr fn;
	std::vector<LemniExpr> args;
};

struct LemniLiteralExprT: LemniExprT{ using LemniExprT::LemniExprT; };

struct LemniTupleExprT: LemniLiteralExprT{
	LemniTupleExprT(LemniLocation loc_, std::vector<LemniExpr> elements_)
		: LemniLiteralExprT(loc_), elements(std::move(elements_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	std::vector<LemniExpr> elements;
};

struct LemniConstantExprT: LemniLiteralExprT{ using LemniLiteralExprT::LemniLiteralExprT; };

struct LemniUnitExprT: LemniConstantExprT{
	using LemniConstantExprT::LemniConstantExprT;

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;
};

struct LemniNumExprT: LemniConstantExprT{ using LemniConstantExprT::LemniConstantExprT; };

struct LemniRealExprT: LemniNumExprT{
	LemniRealExprT(LemniLocation loc_, LemniStr str)
		: LemniNumExprT(loc_), val(lemni::toStdStrView(str)){}

	LemniRealExprT(LemniLocation loc_, lemni::AReal val_)
		: LemniNumExprT(loc_), val(std::move(val_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;

	lemni::AReal val;
};

struct LemniRatioExprT: LemniNumExprT{
	LemniRatioExprT(LemniLocation loc_, LemniStr str, const int base = 10)
		: LemniNumExprT(loc_), val(lemni::toStdStrView(str), base){}

	LemniRatioExprT(LemniLocation loc_, lemni::ARatio val_)
		: LemniNumExprT(loc_), val(std::move(val_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;

	lemni::ARatio val;
};

struct LemniIntExprT: LemniNumExprT{
	LemniIntExprT(LemniLocation loc_, LemniStr str, const int base = 10)
		: LemniNumExprT(loc_), val(lemni::toStdStrView(str), base){}

	LemniIntExprT(LemniLocation loc_, lemni::AInt val_)
		: LemniNumExprT(loc_), val(std::move(val_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;

	lemni::AInt val;
};

struct LemniStrExprT: LemniConstantExprT{
	explicit LemniStrExprT(LemniLocation loc_, std::string str)
		: LemniConstantExprT(loc_), val(std::move(str)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;

	std::string val;
};

struct LemniCommaListExprT: LemniExprT{
	explicit LemniCommaListExprT(LemniLocation loc_, std::vector<LemniExpr> elements_)
		: LemniExprT(loc_), elements(std::move(elements_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope) const noexcept override;

	std::vector<LemniExpr> elements;
};

struct LemniUnaryOpExprT: LemniExprT{
	LemniUnaryOpExprT(LemniLocation loc_, LemniUnaryOp op_, LemniExpr expr_)
		: LemniExprT(loc_), op(op_), expr(expr_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniUnaryOp op;
	LemniExpr expr;
};

struct LemniBinaryOpExprT: LemniExprT{
	LemniBinaryOpExprT(LemniLocation loc_, LemniBinaryOp op_, LemniExpr lhs_, LemniExpr rhs_)
		: LemniExprT(loc_), op(op_), lhs(lhs_), rhs(rhs_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniBinaryOp op;
	LemniExpr lhs, rhs;
};

struct LemniLambdaExprT: LemniExprT{
	LemniLambdaExprT(LemniLocation loc_, std::vector<LemniExpr> params_, LemniExpr body_)
		: LemniExprT(loc_), params(std::move(params_)), body(body_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	std::vector<LemniExpr> params;
	LemniExpr body;
};

struct LemniBranchExprT: LemniExprT{
	LemniBranchExprT(LemniLocation loc_, LemniExpr cond_, LemniExpr true__, LemniExpr false__)
		: LemniExprT(loc_), cond(cond_), true_(true__), false_(false__){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniExpr cond, true_, false_;
};

struct LemniReturnExprT: LemniExprT{
	explicit LemniReturnExprT(LemniLocation loc_, LemniExpr expr_)
		: LemniExprT(loc_), expr(expr_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override{
		return expr->typecheck(state, scope);
	}

	LemniExpr expr;
};

struct LemniBlockExprT: LemniExprT{
	explicit LemniBlockExprT(LemniLocation loc_, std::vector<LemniExpr> exprs_)
		: LemniExprT(loc_), exprs(std::move(exprs_)){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	std::vector<LemniExpr> exprs;
};

struct LemniLValueExprT: LemniExprT{
	LemniLValueExprT(LemniLocation loc_, std::string id_) noexcept
		: LemniExprT(loc_), id(std::move(id_)){}

	std::string id;
};

struct LemniRefExprT: LemniLValueExprT{
	using LemniLValueExprT::LemniLValueExprT;

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;
};

struct LemniBindingExprT: LemniLValueExprT{
	LemniBindingExprT(LemniLocation loc_, std::string id_, LemniExpr value_)
		: LemniLValueExprT(loc_, std::move(id_)), value(value_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniExpr value;
};

struct LemniParamBindingExprT: LemniLValueExprT{
	LemniParamBindingExprT(LemniLocation loc_, std::string id_, LemniExpr type_ = nullptr) noexcept
		: LemniLValueExprT(loc_, std::move(id_)), type(type_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	LemniExpr type;
};

struct LemniFnDefExprT: LemniLValueExprT{
	LemniFnDefExprT(LemniLocation loc_, std::string id_, std::vector<LemniParamBindingExpr> params_, LemniExpr body_)
		: LemniLValueExprT(loc_, std::move(id_)), params(std::move(params_)), body(body_){}

	LemniTypecheckResult typecheck(LemniTypecheckState state, LemniScope scope) const noexcept override;

	std::vector<LemniParamBindingExpr> params;
	LemniExpr body;
};

#endif // !LEMNI_LIB_EXPR_HPP
