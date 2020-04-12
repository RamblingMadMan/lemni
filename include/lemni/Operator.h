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
#include <stdbool.h>

/**
 * @defgroup Operators Types and functions related to unary and binary operators.
 * @{
 */

/**
 * @brief Type representing a unary operator.
 */
typedef enum {
	LEMNI_UNARY_NEG,
	LEMNI_UNARY_NOT,

	LEMNI_UNARY_OP_COUNT,
	LEMNI_UNARY_OP_UNRECOGNIZED = LEMNI_UNARY_OP_COUNT
} LemniUnaryOp;

/**
 * @brief Type representing a binary operator.
 */
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

	LEMNI_BINARY_OP_COUNT,
	LEMNI_BINARY_OP_UNRECOGNIZED = LEMNI_BINARY_OP_COUNT
} LemniBinaryOp;

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
