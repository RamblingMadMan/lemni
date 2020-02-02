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

#ifndef LEMNI_OPERATOR_H
#define LEMNI_OPERATOR_H 1

#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
	LEMNI_UNARY_NEG,
	LEMNI_UNARY_NOT,

	LEMNI_UNARY_OP_COUNT
} LemniUnaryOp;

typedef enum {
	LEMNI_BINARY_ADD,
	LEMNI_BINARY_SUB,
	LEMNI_BINARY_MUL,
	LEMNI_BINARY_DIV,
	LEMNI_BINARY_POW,
	LEMNI_BINARY_MOD,

	LEMNI_BINARY_AND,
	LEMNI_BINARY_OR,

	LEMNI_BINARY_EQ,
	LEMNI_BINARY_NEQ,
	LEMNI_BINARY_LT,
	LEMNI_BINARY_LTEQ,
	LEMNI_BINARY_GT,
	LEMNI_BINARY_GTEQ,

	LEMNI_BINARY_OP_COUNT
} LemniBinaryOp;

LemniUnaryOp lemniUnaryOpFromStr(LemniStr str);
LemniBinaryOp lemniBinaryOpFromStr(LemniStr str);

uint32_t lemniBinaryOpPrecedence(LemniBinaryOp op);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_OPERATOR_H
