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

#include "Macros.h"
#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup Operators Types and functions related to unary and binary operators.
 * @{
 */

#define LEMNI_DEF_UNARY_OP(name) LEMNI_UNARY_##name = 1 << (__COUNTER__ - LEMNI_UNARY_OP_BASE),

/**
 * @brief Type representing a unary operator.
 */

typedef enum {
	LEMNI_UNARY_OP_BASE = __COUNTER__ + 1,

	LEMNI_DEF_UNARY_OP(NEG)
	LEMNI_DEF_UNARY_OP(NOT)

	LEMNI_UNARY_OP_COUNT = __COUNTER__ - LEMNI_UNARY_OP_BASE,
	LEMNI_UNARY_OP_UNRECOGNIZED = LEMNI_UNARY_OP_COUNT
} LemniUnaryOp;

#undef LEMNI_DEF_UNARY_OP

#define LEMNI_DEF_BINARY_OP(name) LEMNI_BINARY_##name = 1 << (__COUNTER__ - LEMNI_BINARY_OP_BASE),

/**
 * @brief Type representing a binary operator.
 */
typedef enum {
	LEMNI_BINARY_OP_BASE = __COUNTER__ + 1,

	LEMNI_DEF_BINARY_OP(ADD)
	LEMNI_DEF_BINARY_OP(SUB)
	LEMNI_DEF_BINARY_OP(MUL)
	LEMNI_DEF_BINARY_OP(DIV)
	LEMNI_DEF_BINARY_OP(MOD)
	LEMNI_DEF_BINARY_OP(POW)

	LEMNI_DEF_BINARY_OP(CONCAT)

	LEMNI_DEF_BINARY_OP(AND)
	LEMNI_DEF_BINARY_OP(OR)

	LEMNI_DEF_BINARY_OP(EQ)
	LEMNI_DEF_BINARY_OP(NEQ)
	LEMNI_DEF_BINARY_OP(LT)
	LEMNI_DEF_BINARY_OP(LTEQ)
	LEMNI_DEF_BINARY_OP(GT)
	LEMNI_DEF_BINARY_OP(GTEQ)

	LEMNI_BINARY_OP_COUNT = __COUNTER__ - LEMNI_BINARY_OP_BASE,
	LEMNI_BINARY_OP_UNRECOGNIZED = LEMNI_BINARY_OP_COUNT
} LemniBinaryOp;

#undef LEMNI_DEF_BINARY_OP

/**
 * @brief Get the unary operator represented by \p str .
 * @param str the string to check
 * @returns the unary operator represented by \p str or ``LEMNI_UNARY_OP_UNRECOGNIZED``.
 */
LemniUnaryOp lemniUnaryOpFromStr(LemniStr str);

/**
 * @brief Get the binary operator represented by \p str .
 * @param str the string to check
 * @returns the binary operator represented by \p str or ``LEMNI_BINARY_OP_UNRECOGNIZED``.
 */
LemniBinaryOp lemniBinaryOpFromStr(LemniStr str);

/**
 * @brief Get the precedence of \p op .
 * @param op the operator to check
 * @returns the precedence of \p op or ``UINT32_MAX`` if not recognized.
 */
uint32_t lemniBinaryOpPrecedence(LemniBinaryOp op);

/**
 * @brief Check if a binary operator is logical/boolean
 * @param op the operator to check
 * @returns whether the operator returns a boolean value
 */
bool lemniBinaryOpIsLogic(LemniBinaryOp op);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_OPERATOR_H
