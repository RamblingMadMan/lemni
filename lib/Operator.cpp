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

#include <map>

#define LEMNI_NO_CPP
#include "lemni/Operator.h"

LemniUnaryOp lemniUnaryOpFromStr(LemniStr str){
	static const std::map<std::string, LemniUnaryOp, std::less<void>> opMap{
		{"-", LEMNI_UNARY_NEG},
		{"!", LEMNI_UNARY_NOT}
	};

	auto res = opMap.find(std::string_view(str.ptr, str.len));
	if(res != end(opMap))
		return res->second;

	return LEMNI_UNARY_OP_COUNT;
}

LemniBinaryOp lemniBinaryOpFromStr(LemniStr str){
	static const std::map<std::string, LemniBinaryOp, std::less<void>> opMap{
		{"+", LEMNI_BINARY_ADD},
		{"-", LEMNI_BINARY_SUB},
		{"*", LEMNI_BINARY_MUL},
		{"/", LEMNI_BINARY_DIV},
		{"^", LEMNI_BINARY_POW},
		{"%", LEMNI_BINARY_MOD},

		{"&", LEMNI_BINARY_AND},
		{"|", LEMNI_BINARY_OR},

		{"==", LEMNI_BINARY_EQ},
		{"!=", LEMNI_BINARY_NEQ},
		{"<", LEMNI_BINARY_LT},
		{"<=", LEMNI_BINARY_LTEQ},
		{">", LEMNI_BINARY_GT},
		{">=", LEMNI_BINARY_GTEQ}
	};

	auto res = opMap.find(std::string_view(str.ptr, str.len));
	if(res != end(opMap))
		return res->second;

	return LEMNI_BINARY_OP_COUNT;
}

uint32_t lemniBinaryOpPrecedence(LemniBinaryOp op){
	constexpr uint32_t precBase = __COUNTER__;

#define BINOP_NEXT_PREC (__COUNTER__ - precBase)

	switch(op){
		case LEMNI_BINARY_POW:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_MUL:
		case LEMNI_BINARY_DIV:
		case LEMNI_BINARY_MOD:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_ADD:
		case LEMNI_BINARY_SUB:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_LT:
		case LEMNI_BINARY_LTEQ:
		case LEMNI_BINARY_GT:
		case LEMNI_BINARY_GTEQ:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_EQ:
		case LEMNI_BINARY_NEQ:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_AND:
			return BINOP_NEXT_PREC;

		case LEMNI_BINARY_OR:
			return BINOP_NEXT_PREC;

		default: return UINT32_MAX;
	}

#undef BINOP_NEXT_PREC
}
