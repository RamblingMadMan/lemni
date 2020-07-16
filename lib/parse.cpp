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

#include <iostream>
#include <new>
#include <memory>
#include <vector>

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

const LemniToken *lemniParseStateTokens(LemniParseStateConst state){
	return state->tokens;
}

size_t lemniParseStateNumTokens(LemniParseStateConst state){
	return state->numTokens;
}

namespace {
	inline LemniParseResult makeError(LemniParseState state, LemniLocation loc, std::string msg){
/*
#ifndef NDEBUG
		throw std::runtime_error(msg);
#endif
*/
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

	inline bool isDelimTok(const LemniToken *it, const LemniToken *end){
		if(it == end) return true;
		else{
			return
				(it->type == LEMNI_TOKEN_BRACKET_CLOSE) ||
				(it->type == LEMNI_TOKEN_DEINDENT) ||
				(it->type == LEMNI_TOKEN_NEWLINE);
		}
	}

	inline const LemniToken *skipWs(const LemniToken *it, const LemniToken *end){
		while((it != end) && (it->type == LEMNI_TOKEN_SPACE)) ++it;
		return it;
	}

	std::pair<LemniParseResult, const LemniToken*> parseInner(LemniParseState state, const LemniToken *it, const LemniToken *const end);
	std::pair<LemniParseResult, const LemniToken*> parseLeading(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, LemniExpr value);

	// starts at first token after '('
	std::pair<LemniParseResult, const LemniToken*> parseParenInner(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end){
		it = skipWs(it, end);

		if(it == end){
			return std::make_pair(makeError(state, loc, "Unexpected end of tokens in paren expression"), it);
		}

		auto valueRet = parseInner(state, it, end);

		it = valueRet.second;

		if(valueRet.first.hasError)
			return valueRet;
		else if(it == end)
			return std::make_pair(makeError(state, loc, "Unexpected end of tokens in paren expression"), it);
		else if(it->text != LEMNICSTR(")"))
			return std::make_pair(makeError(state, loc, "Unexpected delimiter '" + lemni::toStdStr(it->text) + "' in paren expression"), it);

		auto headExpr = valueRet.first.expr;

		std::vector<LemniExpr> elements;

		if(auto list = lemniExprAsCommaList(headExpr)){
			elements.reserve(list->elements.size());
			elements.insert(begin(elements), cbegin(list->elements), cend(list->elements));
		}
		else if(headExpr){
			auto lit = lemniExprAsLiteral(headExpr);

			if(auto tuple = lemniLiteralExprAsTuple(lit)){
				elements.reserve(tuple->elements.size());
				elements.insert(begin(elements), cbegin(tuple->elements), cend(tuple->elements));
			}
			else{
				elements.emplace_back(headExpr);
			}
		}

		auto tupleExpr = createExpr<LemniTupleExprT>(state, loc, std::move(elements));

		auto delimIt = it;

		setRemainder(state, ++it, end);

		return std::make_pair(makeResult(tupleExpr), delimIt);
		//return parseLeading(state, loc, ++delimIt, end, tupleExpr);
	}

	std::pair<LemniParseResult, const LemniToken*> parseFnDef(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *idTok, LemniExpr parenExpr){
		it = skipWs(it, end);

		if(it == end){
			return std::make_pair(makeError(state, LemniLocation{UINT32_MAX, UINT32_MAX}, "Unexpected end of of tokens after function definition parameters"), it);
		}
		else if(it->type == LEMNI_TOKEN_NEWLINE){
			// TODO: parse multi-line parameters
			return std::make_pair(makeError(state, it->loc, "Unexpected end of line after function definition parameters"), it);
		}
		else if(it->text != LEMNICSTR("=")){
			return std::make_pair(makeError(state, it->loc, "Expected assignment after function parameters"), it);
		}

		++it; // skip assignment token

		std::vector<LemniParamBindingExpr> paramExprs;

		auto parenTupExpr = lemniLiteralExprAsTuple(lemniExprAsLiteral(parenExpr));

		paramExprs.reserve(parenTupExpr->elements.size());

		for(auto param : parenTupExpr->elements){
			if(auto ref = dynamic_cast<LemniRefExpr>(param)){
				auto paramExpr = createExpr<LemniParamBindingExprT>(state, ref->loc, ref->id);
				paramExprs.emplace_back(paramExpr);
			}
			else{
				return std::make_pair(makeError(state, param->loc, "Unexpected expression for function parameter"), it);
			}
		}

		it = skipWs(it, end);

		if(it == end){
			return std::make_pair(makeError(state, LemniLocation{UINT32_MAX, UINT32_MAX}, "Unexpected end of of tokens after function assignment"), it);
		}

		bool indented = false;

		if(it->type == LEMNI_TOKEN_NEWLINE){
			++it;

			if(it->type != LEMNI_TOKEN_INDENT){
				return std::make_pair(makeError(state, it->loc, "Expected indentation before body of function"), it);
			}

			indented = true;
		}

		LemniExpr bodyExpr = nullptr;

		if(indented){
			std::vector<LemniExpr> body;

			auto blockLoc = it->loc;

			while(1){
				auto innerRes = parseInner(state, it, end);

				if(innerRes.first.hasError)
					return innerRes;

				body.emplace_back(innerRes.first.expr);

				it = innerRes.second;

				if(it != end){
					if(it->type == LEMNI_TOKEN_NEWLINE){
						++it;
						if(it->type == LEMNI_TOKEN_DEINDENT){
							break;
						}
					}
					else{
						break;
					}
				}
				else{
					break;
				}
			}

			if(body.size() == 1){
				bodyExpr = body.back();
			}
			else{
				auto block = createExpr<LemniBlockExprT>(state, blockLoc, std::move(body));
				bodyExpr = block;
			}
		}
		else{
			auto innerRes = parseInner(state, it, end);

			if(innerRes.first.hasError)
				return innerRes;

			bodyExpr = innerRes.first.expr;

			it = innerRes.second;
		}

		auto fnDef = createExpr<LemniFnDefExprT>(state, idTok->loc, std::string(idTok->text.ptr, idTok->text.len), std::move(paramExprs), bodyExpr);

		auto delimIt = it;
		if(it != end) ++it;

		setRemainder(state, it, end);

		return std::make_pair(makeResult(fnDef), delimIt);
	}

	std::pair<LemniParseResult, const LemniToken*> parseId(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *idTok){
		if(it == end){
			if(idTok->text == LEMNICSTR("import")){
				return std::make_pair(makeError(state, idTok->loc, "unexpected end of tokens in import expression"), it);
			}

			auto ref = createExpr<LemniRefExprT>(state, idTok->loc, std::string(idTok->text.ptr, idTok->text.len));
			setRemainder(state, it, end);
			return std::make_pair(makeResult(ref), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			if(idTok->text == LEMNICSTR("import")){
				return std::make_pair(makeError(state, idTok->loc, "can not define a function with the name 'import'"), it);
			}

			auto parenIt = it;
			auto parenRes = parseParenInner(state, parenIt->loc, ++it, end);
			if(parenRes.first.hasError) return parenRes;

			return parseFnDef(state, parenRes.second + 1, end, idTok, parenRes.first.expr);
		}
		else{
			auto ref = createExpr<LemniRefExprT>(state, idTok->loc, std::string(idTok->text.ptr, idTok->text.len));
			return parseLeading(state, idTok->loc, it, end, ref);
		}
	}

	std::pair<LemniParseResult, const LemniToken*> parseInt(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *intTok){
		int base = 10;
		LemniStr str = intTok->text;

		switch(intTok->type){
			case LEMNI_TOKEN_NAT: break;

			case LEMNI_TOKEN_HEX:{
				base = 16;
				break;
			}

			case LEMNI_TOKEN_OCTAL:{
				base = 8;
				break;
			}

			case LEMNI_TOKEN_BINARY:{
				base = 2;
				break;
			}

			default: return std::make_pair(makeError(state, it->loc, "Unknown integer token type"), it);
		}

		if(base != 10){
			str = lemniSubStr(intTok->text, 2, intTok->text.len - 2);
		}

		LemniIntExprT *int_ = createExpr<LemniIntExprT>(state, intTok->loc, str, base);

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(int_), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function names must start with an alphabetic character or underscore"), it);
		}
		else
			return parseLeading(state, intTok->loc, it, end, int_);
	}

	std::pair<LemniParseResult, const LemniToken*> parseReal(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *realTok){
		auto real = createExpr<LemniRealExprT>(state, realTok->loc, realTok->text);

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(real), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function names must start with an alphabetic character or underscore"), it);
		}
		else{
			return parseLeading(state, realTok->loc, it, end, real);
		}
	}

	std::pair<LemniParseResult, const LemniToken*> parseStr(LemniParseState state, const LemniToken *it, const LemniToken *const end, const LemniToken *strTok){
		auto str = createExpr<LemniStrExprT>(state, strTok->loc, lemni::toStdStr(strTok->text));

		if(it == end){
			setRemainder(state, it, end);
			return std::make_pair(makeResult(str), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Function names must start with an alphabetic character or underscore"), it);
		}
		else{
			return parseLeading(state, strTok->loc, it, end, str);
		}
	}

	// Ordering/BEDMAS will be done in the typecheck phase
	std::pair<LemniParseResult, const LemniToken*> parseBinop(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, LemniExpr lhs, const LemniToken *opTok){
		if(it == end){
			return std::make_pair(makeError(state, opTok->loc, "Unexpected end of tokens after binary operator"), it);
		}

		LemniBinaryOp op = lemniBinaryOpFromStr(opTok->text);
		if(op == LEMNI_BINARY_OP_UNRECOGNIZED){
			return std::make_pair(makeError(state, opTok->loc, "Unrecognized binary operator"), it);
		}

		if(it->type == LEMNI_TOKEN_NEWLINE){
			return std::make_pair(makeError(state, it->loc, "Unexpected end of expression after binary operator"), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_CLOSE){
			return std::make_pair(makeError(state, it->loc, "Unexpected closing bracket after operator"), it);
		}
		else if(it->type == LEMNI_TOKEN_BRACKET_OPEN){
			return std::make_pair(makeError(state, it->loc, "Unexpected opening bracket without space after operator"), it);
		}
		else if(it->type == LEMNI_TOKEN_SPACE){
			do {
				++it;
			} while((it != end) && (it->type == LEMNI_TOKEN_SPACE));

			if(it->type == LEMNI_TOKEN_NEWLINE){
				return std::make_pair(makeError(state, it->loc, "Unexpected end of expression after binary operator"), it);
			}
			else if(it->type == LEMNI_TOKEN_BRACKET_CLOSE){
				return std::make_pair(makeError(state, it->loc, "Unexpected closing bracket after operator"), it);
			}
		}

		auto rhsRet = parseInner(state, it, end);
		if(rhsRet.first.hasError)
			return rhsRet;

		auto binaryOp = createExpr<LemniBinaryOpExprT>(state, loc, op, lhs, rhsRet.first.expr);

		return std::make_pair(makeResult(binaryOp), rhsRet.second);
	}

	std::pair<LemniParseResult, const LemniToken*> parseUnaryOp(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, const LemniToken *opTok){
		while((it != end) && (it->type == LEMNI_TOKEN_SPACE)){
			++it;
		}

		if(it == end){
			return std::make_pair(makeError(state, loc, "Unexpected end of tokens after unary operator"), opTok);
		}

		LemniUnaryOp unaryOp = lemniUnaryOpFromStr(opTok->text);
		if(unaryOp == LEMNI_UNARY_OP_UNRECOGNIZED){
			return std::make_pair(makeError(state, loc, "Invalid unary op"), opTok);
		}

		auto rhsRet = parseInner(state, it, end);

		if(rhsRet.first.hasError)
			return rhsRet;

		if(auto rhsBinaryOp = lemniExprAsBinaryOp(rhsRet.first.expr)){
			auto unaryOpExpr = createExpr<LemniUnaryOpExprT>(state, loc, unaryOp, rhsBinaryOp->lhs);
			auto resultExpr = createExpr<LemniBinaryOpExprT>(state, loc, rhsBinaryOp->op, unaryOpExpr, rhsBinaryOp->rhs);
			return std::make_pair(makeResult(resultExpr), rhsRet.second);
		}
		else{
			auto unaryOpExpr = createExpr<LemniUnaryOpExprT>(state, loc, unaryOp, rhsRet.first.expr);
			return std::make_pair(makeResult(unaryOpExpr), rhsRet.second);
		}
	}

	std::pair<LemniParseResult, const LemniToken*> parseApplication(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, LemniExpr fn){
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
		else if(auto argsBinop = lemniExprAsBinaryOp(argsExpr)){
			args.emplace_back(argsBinop->lhs);
			auto app = createExpr<LemniApplicationExprT>(state, loc, fn, std::move(args));
			auto ret = createExpr<LemniBinaryOpExprT>(state, loc, argsBinop->op, app, argsBinop->rhs);
			return std::make_pair(makeResult(ret), delimIt);
		}
		else{
			args.emplace_back(argsExpr);
		}

		auto app = createExpr<LemniApplicationExprT>(state, loc, fn, std::move(args));

		return std::make_pair(makeResult(app), delimIt);
	}

	// starts on first token after ','
	std::pair<LemniParseResult, const LemniToken*> parseCommaList(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, LemniExpr head){
		std::vector<LemniExpr> elems{head};

		if(it == end){
			return std::make_pair(makeError(state, loc, "Unexpected end of tokens in comma list"), it);
		}

		auto startIt = it;

		while((it != end) && (it->type == LEMNI_TOKEN_SPACE)){
			++it;
		}

		if(it == end){
			return std::make_pair(makeError(state, startIt->loc, "Unexpected end of tokens in comma-separated list"), it);
		}

		auto tailRes = parseInner(state, it, end);

		if(tailRes.first.hasError)
			return tailRes;

		it = tailRes.second;

		if(auto list = lemniExprAsCommaList(tailRes.first.expr)){
			elems.insert(begin(elems), cbegin(list->elements), cend(list->elements));
		}
		else{
			elems.emplace_back(tailRes.first.expr);
		}

		while((it != end) && (it->type == LEMNI_TOKEN_NEWLINE)){
			auto newIt = it;
			++newIt;

			if((newIt != end) && (newIt->type == LEMNI_TOKEN_INDENT)){
				// indented comma list
				it = newIt;
				++newIt;

				if(newIt != end){
					do {
						++it;

						if((it != end) && (it->text == LEMNICSTR(","))){
							tailRes = parseInner(state, ++it, end);

							if(tailRes.first.hasError)
								return tailRes;

							it = tailRes.second;

							if(auto list = lemniExprAsCommaList(tailRes.first.expr)){
								elems.insert(cend(elems), cbegin(list->elements), cend(list->elements));
							}
							else{
								elems.emplace_back(tailRes.first.expr);
							}
						}
						else{
							return std::make_pair(makeError(state, it->loc, "Unexpected token in comma-separated list"), it);
						}
					} while((it != end) && (it->type == LEMNI_TOKEN_NEWLINE));
				}
				else{
					break;
				}
			}
			else{
				break;
			}
		}

		auto list = createExpr<LemniCommaListExprT>(state, loc, std::move(elems));

		setRemainder(state, it, end);

		return std::make_pair(makeResult(list), it);
	}

	std::pair<LemniParseResult, const LemniToken*> parseLeading(LemniParseState state, LemniLocation loc, const LemniToken *it, const LemniToken *const end, LemniExpr value){
		while((it != end) && (it->type == LEMNI_TOKEN_SPACE)){
			++it;
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
		else if(it->type == LEMNI_TOKEN_BRACKET_CLOSE){
			setRemainder(state, it+1, end);
			return std::make_pair(makeResult(value), it);
		}
		else if(it->text == LEMNICSTR(",")){
			return parseCommaList(state, loc, ++it, end, value);
		}
		else if(it->type == LEMNI_TOKEN_OP){
			auto opTok = it;
			return parseBinop(state, loc, ++it, end, value, opTok);
		}
		else{
			return parseApplication(state, loc, it, end, value);
		}
	}

	std::pair<LemniParseResult, const LemniToken*> parseInner(LemniParseState state, const LemniToken *it, const LemniToken *const end){
		if(it == end){
			return std::make_pair(makeResult(nullptr), it);
		}

		switch(it->type){
			case LEMNI_TOKEN_BRACKET_OPEN:{
				if(it->text == LEMNICSTR("(")){
					auto parenIt = it;
					auto parenRet = parseParenInner(state, parenIt->loc, ++it, end);
					if(parenRet.first.hasError){
						return parenRet;
					}
					else{
						it = parenRet.second;
						return parseLeading(state, parenIt->loc, ++it, end, parenRet.first.expr);
					}
				}
				else{
					return std::make_pair(makeError(state, it->loc, "Unexpected bracket token"), it);
				}
			}

			case LEMNI_TOKEN_BRACKET_CLOSE:{
				return std::make_pair(makeResult(nullptr), it);
			}

			case LEMNI_TOKEN_INDENT:{
				return std::make_pair(makeError(state, it->loc, "Unexpected indentation"), it);
			}

			case LEMNI_TOKEN_DEINDENT:{
				return std::make_pair(makeResult(nullptr), it);
			}

			case LEMNI_TOKEN_ID:{
				auto idTok = it;
				return parseId(state, ++it, end, idTok);
			}

			case LEMNI_TOKEN_NAT:
			case LEMNI_TOKEN_HEX:
			case LEMNI_TOKEN_OCTAL:
			case LEMNI_TOKEN_BINARY:{
				auto intTok = it;
				return parseInt(state, ++it, end, intTok);
			}

			case LEMNI_TOKEN_REAL:{
				auto realTok = it;
				return parseReal(state, ++it, end, realTok);
			}

			case LEMNI_TOKEN_OP:{
				auto opTok = it;
				return parseUnaryOp(state, opTok->loc, ++it, end, opTok);
			}

			case LEMNI_TOKEN_STR:{
				auto strTok = it;
				return parseStr(state, ++it, end, strTok);
			}

			default:
				return std::make_pair(makeError(state, it->loc, "Parser mostly unimplemented, sorry :^/"), it);
		}
	}
}

LemniParseResult lemniParse(LemniParseState state){
	auto it = state->tokens;
	auto end = state->tokens + state->numTokens;

	if(it == end) return makeResult(nullptr);

	auto ret = parseInner(state, it, end);

	if(ret.first.hasError)
		return ret.first;
	else if(ret.second == end)
		return ret.first;
	else if(ret.second->type == LEMNI_TOKEN_BRACKET_CLOSE)
		return makeError(state, ret.second->loc, "Unexpected closing bracket");
	else if(ret.second->type == LEMNI_TOKEN_DEINDENT)
		return makeError(state, ret.second->loc, "Unexpected deindent");
	else
		return ret.first;
}
