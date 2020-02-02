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

#include "lemni/parse.h"

struct LemniParseStateT{
	const LemniToken *tokens;
	size_t numTokens;

	std::vector<std::unique_ptr<std::string>> errStrs;
};

LemniParseState lemniCreateParseState(const LemniToken *const tokens, const size_t n){
	auto mem = std::malloc(sizeof(LemniParseStateT));
	auto p = new(mem) LemniParseStateT;

	p->tokens = tokens;
	p->numTokens = n;

	return p;
}

void lemniDestroyParseState(LemniParseState state){
	std::destroy_at(state);
	std::free(state);
}

const LemniToken *lemniParseStateTokens(LemniParseState state){
	return state->tokens;
}

size_t lemniParseStateNumTokens(LemniParseState state){
	return state->numTokens;
}

namespace {
	inline LemniParseResult makeError(LemniParseState state, LemniLocation loc, std::string msg){
		auto &&str = state->errStrs.emplace_back(std::make_unique<std::string>(std::move(msg)));
		LemniParseResult ret;
		ret.hasError = true;
		ret.error = { .loc = loc, .msg = { .ptr = str->c_str(), .len = str->size() } };
		return ret;
	}

	inline LemniParseResult makeResult(LemniExpr expr){
		LemniParseResult ret;
		ret.hasError = false;
		ret.expr = expr;
		return ret;
	}

	LemniParseResult parseInner(LemniParseState state, )
}

LemniParseResult lemniParse(LemniParseState state){
	auto it = state->tokens;
	auto end = state->tokens + state->numTokens;

	if(it == end){
		return makeResult(nullptr);
	}
	else if(it->type == LEMNI_TOKEN_INDENT){
		return makeError(state, it->loc, "Unexpected indentation");
	}

	return makeError(state, it->loc, "Parsing unimplemented");
}
