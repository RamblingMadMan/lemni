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

#ifndef LEMNI_LEX_H
#define LEMNI_LEX_H 1

#include "Macros.h"
#include "Token.h"

/**
 * @defgroup Lexing Lexing related types and functions.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque type representing lex state.
 */

LEMNI_OPAQUE_T(LemniLexState);

typedef struct LemniLexStateResultT{
	bool hasError;
	union {
		LemniLexState state;
		LemniStr errMsg;
	};
} LemniLexStateResult;

/**
 * @brief Type representing a lexing error.
 */
typedef struct {
	LemniLocation loc;
	LemniStr msg;
} LemniLexError;

/**
 * @brief Type representing the result of a lexing operation.
 */
typedef struct {
	bool hasError;
	union {
		LemniLexError error;
		LemniToken token;
	};
} LemniLexResult;

/**
 * @brief Create new state for lexing operations.
 * @note the returned state must be destroyed with \ref lemniDestroyLexState .
 * @warning the string \p str must stay valid for the life of the returned state.
 * @param str the string to lex
 * @param startLoc where the first location should be recorded
 * @returns the newly created state
 */
LemniLexState lemniCreateLexState(LemniStr str, LemniLocation startLoc);

/**
 * @brief Destroy state previously created with \ref lemniCreateLexState .
 * @warning ``NULL`` must not be passed to this function
 * @param state the state to destroy
 */
void lemniDestroyLexState(LemniLexState state);

/**
 * @brief Get the remainder of the string in \p state .
 * @param state the state to check
 * @returns the remaining lex state string
 */
LemniStr lemniLexStateRemainder(LemniLexStateConst state);

/**
 * @brief Get the location that will be given to the next token lexed in \p state .
 * @param state the state to check
 * @returns the next token location
 */
LemniLocation lemniLexStateNextLocation(LemniLexStateConst state);

/**
 * @brief Lex a single token from \p state .
 * @param state the state to modify
 * @returns the result of the parsing operation
 */
LemniLexResult lemniLex(LemniLexState state);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <limits>
#include <string_view>
#include <variant>
#include <vector>

namespace lemni{
	using LexError = LemniLexError;

	class LexState{
		public:
			explicit LexState(std::string_view str, LemniLocation startLoc = LemniLocation{0, 0})
				: m_state(lemniCreateLexState(LemniStr{str.data(), str.size()}, startLoc)){}

			explicit LexState(LemniStr str, LemniLocation startLoc = LemniLocation{0, 0})
				: LexState(lemni::toStdStrView(str), startLoc){}

			LexState(LexState &&other) noexcept
				: m_state(other.m_state)
			{
				other.m_state = nullptr;
			}

			LexState(const LexState&) = delete;

			~LexState(){ if(m_state) lemniDestroyLexState(m_state); }

			LexState &operator=(LexState &&other) noexcept{
				m_state = other.m_state;
				other.m_state = nullptr;
				return *this;
			}

			LexState &operator=(const LexState&) = delete;

			operator LemniLexState() noexcept{ return m_state; }

			LemniLexState handle() noexcept{ return m_state; }

			std::string_view remainder() const noexcept{
				if(!m_state) return {};
				auto str = lemniLexStateRemainder(m_state);
				return {str.ptr, str.len};
			}

			Location nextLocation() const noexcept{
				using pos_t = decltype(Location::col);
				if(!m_state) return { std::numeric_limits<pos_t>::max(), std::numeric_limits<pos_t>::max() };
				return lemniLexStateNextLocation(m_state);
			}

		private:
			LemniLexState m_state;

			friend std::variant<Token, LexError> lex(LexState &state) noexcept;
	};

	inline std::variant<Token, LexError> lex(LexState &state) noexcept{
		auto res = lemniLex(state.m_state);
		if(res.hasError) return res.error;
		else return res.token;
	}

	inline std::variant<std::vector<Token>, LexError> lexAll(std::string_view str){
		LexState state{str};

		std::vector<Token> ret;

		while(1){
			auto res = lex(state);
			if(auto err = std::get_if<LexError>(&res)){
				return *err;
			}
			else{
				auto &&tok = std::get<Token>(res);
				if(tok.type == LEMNI_TOKEN_EOF)
					break;
				else
					ret.emplace_back(std::move(tok));
			}
		}

		return ret;
	}

	inline decltype(auto) lexAll(LemniStr str){ return lexAll(lemni::toStdStrView(str)); }
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_LEX_H
