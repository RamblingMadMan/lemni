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

#ifndef LEMNI_PARSE_H
#define LEMNI_PARSE_H 1

#include "Token.h"
#include "Expr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Parsing Parsing related types and functions.
 * @{
 */

/**
 * @brief Opaque type representing parse state.
 */
typedef struct LemniParseStateT *LemniParseState;

/**
 * @brief Type representing a parsing error.
 */
typedef struct {
	LemniLocation loc;
	LemniStr msg;
} LemniParseError;

/**
 * @brief Type representing the result of a parsing operation.
 */
typedef struct {
	bool hasError;
	union {
		LemniParseError error;
		LemniExpr expr;
	};
} LemniParseResult;

/**
 * @brief Create new state for parsing operations.
 * @note the returned state must be destroyed with \ref lemniDestroyParseState .
 * @warning the pointer \p tokens must stay valid for the life of the returned state.
 * @param tokens pointer to an array of tokens to parse
 * @param n how many tokens in \p tokens array
 * @returns the newly created state
 */
LemniParseState lemniCreateParseState(const LemniToken *const tokens, const size_t n);

/**
 * @brief Destroy state previously created with \ref lemniCreateParseState .
 * @warning ``NULL`` must not be passed to this function.
 * @param state the state to destroy
 */
void lemniDestroyParseState(LemniParseState state);

/**
 * @brief Get a pointer to the remaining tokens in \p state .
 * @param state the state to check
 * @returns pointer to tokens
 */
const LemniToken *lemniParseStateTokens(LemniParseState state);

/**
 * @brief Get the number of remaining tokens in \p state .
 * @param state the state to check
 * @returns the number of tokens
 */
size_t lemniParseStateNumTokens(LemniParseState state);

/**
 * @brief Parse a single expression from \p state .
 * @param state the state to modify
 * @returns the result of the parsing operation
 */
LemniParseResult lemniParse(LemniParseState state);

/**
 * @}
 */

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <variant>

namespace lemni{
	using Expr = LemniExpr;
	using ParseError = LemniParseError;

	class ParseState{
		public:
			ParseState(const LemniToken *const tokens, const size_t n)
				: m_state(lemniCreateParseState(tokens, n)){}

			~ParseState(){ lemniDestroyParseState(m_state); }

		private:
			LemniParseState m_state;

			friend std::variant<Expr, ParseError> parse(ParseState &state);
	};

	inline std::variant<Expr, ParseError> parse(ParseState &state){
		auto res = lemniParse(state.m_state);
		if(res.hasError) return res.error;
		else return res.expr;
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_PARSE_H
