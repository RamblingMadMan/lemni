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

#define LEMNI_NO_CPP
#include "lemni/parse.h"

#include "Expr.hpp"

struct LemniParseStateT{
	const LemniToken *tokens;
	size_t numTokens;

	std::vector<std::unique_ptr<LemniExprT>> exprs;
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

	template<typename T, typename ... Args>
	inline T *createExpr(LemniParseState state, Args &&... args){
		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		auto ret = ptr.get();
		auto insertIt = std::lower_bound(begin(state->exprs), end(state->exprs), ptr);
		state->exprs.insert(insertIt, std::move(ptr));
		return ret;
	}

	inline void setRemainder(LemniParseState state, const LemniToken *const beg, const LemniToken *const end){
		auto numTokens = static_cast<size_t>(std::distance(beg, end));
		state->tokens = beg;
		state->numTokens = numTokens;
	}

	std::pair<LemniParseResult, const LemniToken*> parseInner(LemniParseState state, const LemniToken *it, const LemniToken *const end);
	std::pair<LemniParseResult, const LemniToken*> parseLeading(LemniParseState state, const LemniToken *it, const LemniToken *const end, LemniExpr value);

	std::pair<LemniParseResult, const LemniToken*> parseId(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *idTok){
		auto ref = createExpr<LemniRefExprT>(state, std::string(idTok->text.ptr, idTok->text.len));

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(ref), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function parsing currently unimplemented"), it);
		}
		else
			return parseLeading(state, it, end, ref);
	}

	std::pair<LemniParseResult, const LemniToken*> parseInt(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *intTok){
		auto int_ = createExpr<LemniIntExprT>(state, intTok->text, 10);

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(int_), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function parsing currently unimplemented"), it);
		}
		else
			return parseLeading(state, it, end, int_);
	}

	std::pair<LemniParseResult, const LemniToken*> parseReal(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *realTok){
		auto real = createExpr<LemniRealExprT>(state, realTok->text, 10);

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(real), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function parsing currently unimplemented"), it);
		}
		else
			return parseLeading(state, it, end, real);
	}

	std::pair<LemniParseResult, const LemniToken*> parseBinop(LemniParseState state, const LemniToken *it, const LemniToken *const end, LemniExpr value, LemniBinaryOp op){
		return std::make_pair(makeError(state, it->loc, "Binary operator parsing currently unimplemented"), it);
	}

	std::pair<LemniParseResult, const LemniToken*> parseApplication(LemniParseState state, const LemniToken *it, const LemniToken *const end, LemniExpr fn){
		auto argsRet = parseInner(state, it, end);

		if(argsRet.first.hasError)
			return argsRet;

		auto delimIt = argsRet.second;
		auto argsExpr = argsRet.first.expr;

		std::vector<LemniExpr> args;

		if(auto argsApp = lemniExprAsApplication(argsExpr)){
			args.reserve(argsApp->args.size() + 1);
			args.emplace_back(argsApp->fn);
			for(auto expr : argsApp->args)
				args.emplace_back(expr);
		}
		else{
			args.emplace_back(argsExpr);
		}

		auto app = createExpr<LemniApplicationExprT>(state, fn, std::move(args));

		return std::make_pair(makeResult(app), delimIt);
	}

	std::pair<LemniParseResult, const LemniToken*> parseLeading(LemniParseState state, const LemniToken *it, const LemniToken *const end, LemniExpr value){
		if((it != end) && (it->type == LEMNI_TOKEN_SPACE)){
			do{
				++it;
			} while((it != end) && (it->type == LEMNI_TOKEN_SPACE));
		}

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(value), it);
		}
		else if(it->type == LEMNI_TOKEN_NEWLINE){
			auto newlineIt = it;
			setRemainder(state, ++it, end);
			return std::make_pair(makeResult(value), newlineIt);
		}
		else if(it->type == LEMNI_TOKEN_OP){
			LemniBinaryOp op = lemniBinaryOpFromStr(it->text);
			if(op == LEMNI_BINARY_OP_UNRECOGNIZED){
				return std::make_pair(makeError(state, it->loc, "Unrecognized binary operator"), it);
			}

			return parseBinop(state, ++it, end, value, op);
		}
		else{
			return parseApplication(state, it, end, value);
		}
	}

	std::pair<LemniParseResult, const LemniToken*> parseInner(LemniParseState state, const LemniToken *it, const LemniToken *const end){
		if(it == end){
			return std::make_pair(makeResult(nullptr), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_CLOSE){
			return std::make_pair(makeError(state, it->loc, "Unexpected closing bracket"), it);
		}
		else if(it->type == LEMNI_TOKEN_INDENT){
			return std::make_pair(makeError(state, it->loc, "Unexpected indentation"), it);
		}
		else if(it->type == LEMNI_TOKEN_DEINDENT){
			return std::make_pair(makeResult(nullptr), it);
		}
		else if(it->type == LEMNI_TOKEN_ID){
			auto idTok = it;
			return parseId(state, ++it, end, idTok);
		}
		else if(it->type == LEMNI_TOKEN_INT){
			auto intTok = it;
			return parseInt(state, ++it, end, intTok);
		}
		else if(it->type == LEMNI_TOKEN_REAL){
			auto realTok = it;
			return parseReal(state, ++it, end, realTok);
		}
		else
			return std::make_pair(makeError(state, it->loc, "Parser mostly unimplemented, sorry :^/"), it);
	}
}

LemniParseResult lemniParse(LemniParseState state){
	auto it = state->tokens;
	auto end = state->tokens + state->numTokens;

	auto ret = parseInner(state, it, end);
	if(ret.first.hasError)
		return ret.first;
	else if(ret.second->type == LEMNI_TOKEN_BRACKET_CLOSE)
		return makeError(state, ret.second->loc, "Unexpected closing bracket");
	else if(ret.second->type == LEMNI_TOKEN_DEINDENT)
		return makeError(state, ret.second->loc, "Unexpected deindent");
	else
		return ret.first;
}
