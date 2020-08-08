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

#include "Macros.h"
#include "Token.h"
#include "Expr.h"

/**
 * @defgroup Parsing Parsing related types and functions.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque type representing parse state.
 */
LEMNI_OPAQUE_T(LemniParseState);

/**
 * @brief Type representing a parsing error.
 */
typedef struct {
	LemniLocation loc;
	LemniStr msg;
} LemniParseResultError;

typedef struct {
	LemniExpr expr;
	LemniNat64 numRem;
	const LemniToken *rem;
} LemniParseResultGood;

/**
 * @brief Type representing the result of a parsing operation.
 */
typedef struct {
	bool hasError;
	union {
		LemniParseResultError error;
		LemniParseResultGood res;
	};
} LemniParseResult;

/**
 * @brief Create new state for parsing operations.
 * @note the returned state must be destroyed with \ref lemniDestroyParseState .
 * @returns the newly created state
 */
LemniParseState lemniCreateParseState();

/**
 * @brief Destroy state previously created with \ref lemniCreateParseState .
 * @warning ``NULL`` must not be passed to this function.
 * @param state the state to destroy
 */
void lemniDestroyParseState(LemniParseState state);

/**
 * @brief Parse a single expression from \p state .
 * @param state the state to modify
 * @param numTokens number of tokens in \p tokens array
 * @param tokens pointer to an array of tokens to parse
 * @returns the result of the parsing operation
 */
LemniParseResult lemniParse(LemniParseState state, const LemniNat64 numTokens, const LemniToken *const tokens);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <vector>
#include <variant>

namespace lemni{
	using Expr = LemniExpr;
	using ParseError = LemniParseResultError;

	class ParseState{
		public:
			ParseState()
				: m_state(lemniCreateParseState()){}

			ParseState(ParseState &&other) noexcept
				: m_state(other.m_state)
			{
				other.m_state = nullptr;
			}

			ParseState(const ParseState&) = delete;

			~ParseState(){ if(m_state) lemniDestroyParseState(m_state); }

			ParseState &operator=(ParseState &&other) noexcept{
				if(m_state) lemniDestroyParseState(m_state);
				m_state = other.m_state;
				other.m_state = nullptr;
				return *this;
			}

			ParseState &operator=(const ParseState&) = delete;

			operator LemniParseState() noexcept{ return m_state; }

			LemniParseState handle() noexcept{ return m_state; }

		private:
			LemniParseState m_state;

			friend std::variant<std::pair<Expr, const LemniToken*>, ParseError> parse(ParseState &state, const LemniToken *const it, const LemniToken *const end) noexcept;
	};

	inline std::variant<std::pair<Expr, const LemniToken*>, ParseError> parse(ParseState &state, const LemniToken *const it, const LemniToken *const end) noexcept{
		auto res = lemniParse(state.m_state, static_cast<LemniNat64>(end - it), it);
		if(res.hasError) return res.error;
		else return std::make_pair(res.res.expr, res.res.rem);
	}

	inline std::variant<std::vector<Expr>, ParseError> parseAll(ParseState &state, const LemniToken *const beg, const LemniToken *const end){
		std::vector<Expr> exprs;
		exprs.reserve(static_cast<LemniNat64>(end - beg));

		auto it = beg;

		while(it != end){
			auto res = parse(state, it, end);

			if(auto err = std::get_if<ParseError>(&res))
				return *err;
			else{
				auto val = *std::get_if<std::pair<Expr, const LemniToken*>>(&res);
				if(!val.first) break;

				exprs.emplace_back(val.first);
				it = val.second;
			}
		}

		return exprs;
	}

	inline std::variant<std::vector<Expr>, ParseError> parseAll(ParseState &state, const std::vector<LemniToken> &toks){
		return parseAll(state, toks.data(), toks.data() + toks.size());
	}

	inline std::pair<ParseState, std::variant<std::vector<Expr>, ParseError>> parseAll(const std::vector<LemniToken> &toks){
		auto state = ParseState();
		return std::make_pair(std::move(state), parseAll(state, toks));
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_PARSE_H
