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

#include <cstdlib>
#include <new>
#include <memory>
#include <vector>
#include <queue>
#include <iostream>

#define U_CHARSET_IS_UTF8 1
#include "unicode/uchar.h"
#include "unicode/utypes.h"

#include "utf8.h"

#include "lemni/lex.h"

struct LemniLexStateT{
	LemniStr remainder;
	LemniLocation loc;

	bool onNewLine = true;
	std::vector<LemniStr> indents;
	std::queue<LemniToken> backlog;
	std::vector<std::unique_ptr<std::string>> errStrs;
};

LemniLexState lemniCreateLexState(LemniStr str, LemniLocation startLoc){
	auto mem = std::malloc(sizeof(LemniLexStateT));
	auto p = new(mem) LemniLexStateT;

	p->remainder = str;
	p->loc = startLoc;

	return p;
}

void lemniDestroyLexState(LemniLexState state){
	std::destroy_at(state);
	std::free(state);
}

LemniStr lemniLexStateRemainder(LemniLexState state){
	return state->remainder;
}

LemniLocation lemniLexStateNextLocation(LemniLexState state){
	return state->loc;
}

namespace {
	LemniLexResult makeError(LemniLexState state, LemniLocation loc, std::string msg){
		auto &&str = state->errStrs.emplace_back(std::make_unique<std::string>(std::move(msg)));
		LemniLexResult ret;
		ret.hasError = true;
		ret.error = {.loc = loc, .msg = LemniStr{ .ptr = str->c_str(), .len = str->size() }};
		return ret;
	}

	LemniLexResult makeResult(LemniToken token){
		LemniLexResult ret;
		ret.hasError = false;
		ret.token = token;
		return ret;
	}

	LemniLexResult lexReal(LemniLexState state, LemniLocation loc, const char *const beg, const char *it, const char *const end){
		while(it != end){
			auto cp = utf8::peek_next(it, end);
			if(cp == '.')
				return makeError(state, state->loc, "Multiple decimal points in real literal");
			else if(cp != '_'){
				if(!u_isalnum(cp))
					break;
				else if(!u_isxdigit(cp))
					return makeError(state, state->loc, "Invalid digit in real literal");
			}

			++state->loc.col;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = false;

		auto strLen = static_cast<size_t>(std::distance(beg, it));

		return makeResult(LemniToken{.type = LEMNI_TOKEN_REAL, .text = LemniStr{.ptr = beg, .len = strLen}, .loc = loc});
	}

	LemniLexResult lexInt(LemniLexState state, LemniLocation loc, const char *const beg, const char *it, const char *const end){
		while(it != end){
			auto cp = utf8::peek_next(it, end);
			if(cp == '.'){
				++state->loc.col;
				utf8::advance(it, 1, end);
				return lexReal(state, loc, beg, it, end);
			}
			else if(cp != '_'){
				if(!u_isalnum(cp))
					break;
				else if(!u_isxdigit(cp))
					return makeError(state, state->loc, "Invalid digit in integer literal");
			}

			++state->loc.col;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = false;

		auto strLen = static_cast<size_t>(std::distance(beg, it));

		return makeResult(LemniToken{.type = LEMNI_TOKEN_INT, .text = LemniStr{.ptr = beg, .len = strLen}, .loc = loc});
	}
}

LemniLexResult lemniLex(LemniLexState state){
	if(!state->backlog.empty()){
		auto ret = state->backlog.front();
		state->backlog.pop();
		return makeResult(ret);
	}

	auto it = state->remainder.ptr;
	auto end = state->remainder.ptr + state->remainder.len;

	if(it == end)
		return makeResult(
			LemniToken{
				.type = LEMNI_TOKEN_EOF,
				.text = LemniStr{.ptr = nullptr, .len = 0},
				.loc = state->loc
			}
		);

	auto cp = utf8::peek_next(it, end);

	if(cp == '\n'){
		auto newlineLoc = state->loc;

		++state->loc.line;
		state->loc.col = 0;

		utf8::advance(it, 1, end);

		while(it != end){
			cp = utf8::peek_next(it, end);
			if(cp != '\n')
				break;

			++state->loc.line;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = true;

		return makeResult(
			LemniToken{
				.type = LEMNI_TOKEN_NEWLINE,
				.text = LemniStr{.ptr = nullptr, .len = 0},
				.loc = newlineLoc
			});
	}

	if(state->onNewLine && !state->indents.empty()){
		auto indentLoc = state->loc;
		auto indentStrBeg = it;

		if(u_isspace(cp)){
			utf8::advance(it, 1, end);

			++state->loc.col;

			while(it != end){
				cp = utf8::peek_next(it, end);
				if((cp == '\n') || !u_isspace(cp))
					break;

				++state->loc.col;
				utf8::advance(it, 1, end);
			}
		}

		auto indentStrLen = static_cast<size_t>(std::distance(indentStrBeg, it));

		if(indentStrLen > 0){ // check for indent
			auto indentStr = LemniStr{.ptr = indentStrBeg, .len = indentStrLen};

			//std::cout << "Indentation to check: '" << indentStr << "'\n";

			state->onNewLine = false;

			auto indentVecIt = cbegin(state->indents);
			auto indentVecEnd = cend(state->indents);

			auto strIt = indentStr.ptr;
			auto strEnd = strIt + indentStr.len;

			while((indentVecIt != indentVecEnd) && (strIt != strEnd)){
				auto indentIt = indentVecIt->ptr;
				auto indentEnd = indentIt + indentVecIt->len;

				bool doBreak = false;

				while((indentIt != indentEnd) && (strIt != strEnd)){
					auto indentCp = utf8::next(indentIt, indentEnd);
					auto strCp = utf8::next(strIt, strEnd);

					if(indentCp != strCp){
						doBreak = true;
						break;
					}
				}

				if(strIt == strEnd){
					if(indentIt == indentEnd)
						++indentVecIt;

					break;
				}
				else if(doBreak)
					break;

				++indentVecIt;
			}

			auto deindents = static_cast<size_t>(std::distance(indentVecIt, indentVecEnd));

			//std::cout << "Num deindents " << deindents << '\n';

			LemniToken deindentToken;
			deindentToken.type = LEMNI_TOKEN_DEINDENT;
			deindentToken.text = LemniStr{.ptr = nullptr, .len = 0};
			deindentToken.loc = indentLoc;

			if(deindents > 0){
				for(std::size_t i = 1; i < deindents; i++){
					state->backlog.push(deindentToken);
				}

				state->indents.erase(indentVecIt, indentVecEnd);
			}

			if(indentVecIt == cbegin(state->indents))
				strIt = indentStrBeg;

			auto indentLen = static_cast<size_t>(std::distance(strIt, strEnd));

			LemniToken indentToken;
			indentToken.type = LEMNI_TOKEN_INDENT;
			indentToken.text = LemniStr{.ptr = strIt, .len = indentLen};
			indentToken.loc = indentLoc;

			if(indentLen > 0){
				state->indents.emplace_back(indentToken.text);
			}

			auto remLen = std::distance(it, end);
			state->remainder.ptr = it;
			state->remainder.len = static_cast<size_t>(remLen);

			if(deindents && indentLen){
				state->backlog.push(indentToken);

				return makeResult(deindentToken);
			}
			else if(deindents){
				return makeResult(deindentToken);
			}
			else if(indentLen){
				return makeResult(indentToken);
			}
		}
		else{ // else create deindent
			auto remLen = std::distance(it, end);
			state->remainder.ptr = it;
			state->remainder.len = static_cast<size_t>(remLen);

			state->indents.clear();

			return makeResult(
				LemniToken{
					.type = LEMNI_TOKEN_DEINDENT,
					.text = LemniStr{.ptr = nullptr, .len = 0},
					.loc = indentLoc
				});
		}
	}

	if(u_isspace(cp)){
		auto spaceLoc = state->loc;
		auto spaceStrBeg = it;

		utf8::advance(it, 1, end);

		++state->loc.col;

		while(it != end){
			cp = utf8::peek_next(it, end);
			if((cp == '\n') || !u_isspace(cp))
				break;

			++state->loc.col;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		auto spaceStrLen = static_cast<size_t>(std::distance(spaceStrBeg, it));

		auto spaceStr = LemniStr{.ptr = spaceStrBeg, .len = spaceStrLen};

		if(state->onNewLine){
			state->onNewLine = false;

			state->indents.emplace_back(spaceStr);

			return makeResult(
				LemniToken{
					.type = LEMNI_TOKEN_INDENT,
					.text = spaceStr,
					.loc = spaceLoc
				});
		}
		else{
			return makeResult(
				LemniToken{
					.type = LEMNI_TOKEN_SPACE,
					.text = spaceStr,
					.loc = spaceLoc
				});
		}
	}
	else if(u_isdigit(cp)){
		auto beg = it;
		utf8::advance(it, 1, end);
		return lexInt(state, state->loc, beg, it, end);
	}
	else if((cp == '_') || u_hasBinaryProperty(cp, UCHAR_ALPHABETIC)){
		auto idLoc = state->loc;
		auto idStrBeg = it;

		utf8::advance(it, 1, end);

		++state->loc.col;

		while(it != end){
			cp = utf8::peek_next(it, end);
			if((cp != '_') && !u_isalnum(cp))
				break;

			++state->loc.col;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = false;

		auto idStrLen = static_cast<size_t>(std::distance(idStrBeg, it));

		return makeResult(
			LemniToken{
				.type = LEMNI_TOKEN_ID,
				.text = LemniStr{.ptr = idStrBeg, .len = idStrLen},
				.loc = idLoc
			});
	}
	else if(int32_t dir = u_getIntPropertyValue(cp, UCHAR_BIDI_PAIRED_BRACKET_TYPE); dir != U_BPT_NONE){
		bool opening = (dir == U_BPT_OPEN);
		if(!opening){
			if(dir != U_BPT_CLOSE){
				// wtf is this?
				return makeError(state, state->loc, "Unrecognizable bracket character");
			}

			opening = false;
		}

		auto bracketLoc = state->loc;
		auto bracketStrBeg = it;

		utf8::advance(it, 1, end);

		auto bracketStrLen = static_cast<size_t>(std::distance(bracketStrBeg, it));

		auto bracketStr = LemniStr{.ptr = bracketStrBeg, .len = bracketStrLen};

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = false;

		// the matching open/close bracket
		//UChar32 matched = u_charMirror(cp);

		return makeResult(
			LemniToken{
				.type = opening ? LEMNI_TOKEN_BRACKET_OPEN : LEMNI_TOKEN_BRACKET_CLOSE,
				.text = bracketStr,
				.loc = bracketLoc
			});
	}
	else if(u_ispunct(cp) || u_hasBinaryProperty(cp, UCHAR_MATH)){
		auto opLoc = state->loc;
		auto opStrBeg = it;

		utf8::advance(it, 1, end);

		++state->loc.col;

		while(it != end){
			cp = utf8::peek_next(it, end);
			if(!u_ispunct(cp) && !u_hasBinaryProperty(cp, UCHAR_MATH))
				break;

			++state->loc.col;
			utf8::advance(it, 1, end);
		}

		auto remLen = std::distance(it, end);
		state->remainder.ptr = it;
		state->remainder.len = static_cast<size_t>(remLen);

		state->onNewLine = false;

		auto opStrLen = static_cast<size_t>(std::distance(opStrBeg, it));

		return makeResult(
			LemniToken{
				.type = LEMNI_TOKEN_OP,
				.text = LemniStr{.ptr = opStrBeg, .len = opStrLen},
				.loc = opLoc
			});
	}
	/*
	// string
	else if(u_hasBinaryProperty(cp, UCHAR_QUOTATION_MARK)){

	}
	*/
	else if(u_iscntrl(cp)){
		return makeError(state, state->loc, "utf8 control character encountered");
	}
	else{
		return makeError(state, state->loc, "Invalid utf8 character");
	}
}
