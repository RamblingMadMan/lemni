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

typedef struct {
	LemniLocation loc;
	LemniStr msg;
} LemniParseError;

typedef struct {
	bool hasError;
	union {
		LemniParseError error;
		LemniExpr expr;
	};
} LemniParseResult;

typedef struct LemniParseStateT *LemniParseState;

LemniParseState lemniCreateParseState(const LemniToken *const tokens, const size_t n);

void lemniDestroyParseState(LemniParseState state);

const LemniToken *lemniParseStateTokens(LemniParseState state);

size_t lemniParseStateNumTokens(LemniParseState state);

LemniParseResult lemniParse(LemniParseState state);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_PARSE_H
