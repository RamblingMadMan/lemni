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

#include "lemni/AReal.h"
#include "lemni/TypedExpr.h"

#include "Type.hpp"

struct LemniTypedExprT{
	virtual ~LemniTypedExprT() = default;

	virtual LemniType type() const noexcept = 0;
};

struct LemniTypedBinaryOpExprT: LemniTypedExprT{
	LemniTypedBinaryOpExprT(LemniType resultType_, LemniBinaryOp op_, LemniTypedExpr lhs_, LemniTypedExpr rhs_)
		: resultType(resultType_), op(op_), lhs(lhs_), rhs(rhs_){}

	LemniType resultType;
	LemniBinaryOp op;
	LemniTypedExpr lhs;
	LemniTypedExpr rhs;
};

struct LemniTypedApplicationExprT: LemniTypedExprT{
	LemniTypedApplicationExprT(LemniType resultType_, LemniTypedExpr fn_, std::vector<LemniTypedExpr> args_)
		: resultType(resultType_), fn(fn_), args(std::move(args_)){}

	LemniType type() const noexcept override{ return resultType; }

	LemniType resultType;
	LemniTypedExpr fn;
	std::vector<LemniTypedExpr> args;
};

struct LemniTypedLiteralExprT: LemniTypedExprT{};

struct LemniTypedLambdaExprT: LemniTypedLiteralExprT{};

struct LemniTypedConstantExprT: LemniTypedLiteralExprT{};

struct LemniTypedNumExprT: LemniTypedConstantExprT{};

struct LemniTypedNatExprT: LemniTypedNumExprT{
	LemniTypedNatExprT(LemniNatType natType_, lemni::AInt value_)
		: natType(natType_), value(std::move(value_)){}

	LemniNatType type() const noexcept override{ return natType; }

	LemniNatType natType;
	lemni::AInt value;
};

struct LemniTypedIntExprT: LemniTypedNumExprT{
	LemniTypedIntExprT(LemniIntType intType_, lemni::AInt value_)
		: intType(intType_), value(std::move(value_)){}

	LemniIntType type() const noexcept override{ return intType; }

	LemniIntType intType;
	lemni::AInt value;
};

struct LemniTypedRatioExprT: LemniTypedNumExprT{
	LemniTypedRatioExprT(LemniRatioType ratioType_, lemni::ARatio value_)
		: ratioType(ratioType_), value(std::move(value_)){}

	LemniRatioType type() const noexcept override{ return ratioType; }

	LemniRatioType ratioType;
	lemni::ARatio value;
};

struct LemniTypedRealExprT: LemniTypedNumExprT{
	LemniTypedRealExprT(LemniRealType realType_, lemni::AReal value_)
		: realType(realType_), value(std::move(value_)){}

	LemniRealType type() const noexcept override{ return realType; }

	LemniRealType realType;
	lemni::AReal value;
};

struct LemniTypedStringExprT: LemniTypedConstantExprT{};

struct LemniTypedStringASCIIExprT: LemniTypedStringExprT{
	LemniTypedStringASCIIExprT(LemniStringASCIIType strType_, std::string value_)
		: strType(strType_), value(std::move(value_)){}

	LemniStringASCIIType type() const noexcept override{ return strType; }

	LemniStringASCIIType strType;
	std::string value;
};

struct LemniTypedStringUTF8ExprT: LemniTypedStringExprT{
	LemniTypedStringUTF8ExprT(LemniStringUTF8Type strType_, std::string value_)
		: strType(strType_), value(std::move(value_)){}

	LemniStringUTF8Type type() const noexcept override{ return strType; }

	LemniStringUTF8Type strType;
	std::string value;
};

struct LemniTypedLValueExprT: LemniTypedExprT{};

struct LemniTypedFnDefExprT: LemniTypedLValueExprT{};

#endif // !LEMNI_LIB_TYPEDEXPR_HPP
