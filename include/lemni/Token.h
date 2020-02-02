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

#ifndef LEMNI_TOKEN_H
#define LEMNI_TOKEN_H 1

#include "Str.h"
#include "Location.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LEMNI_TOKEN_SPACE,
	LEMNI_TOKEN_NEWLINE,
	LEMNI_TOKEN_INDENT,
	LEMNI_TOKEN_DEINDENT,

	LEMNI_TOKEN_ID,
	LEMNI_TOKEN_INT,
	LEMNI_TOKEN_REAL,
	LEMNI_TOKEN_OP,
	LEMNI_TOKEN_BRACKET_OPEN,
	LEMNI_TOKEN_BRACKET_CLOSE,
	LEMNI_TOKEN_STR,

	LEMNI_TOKEN_EOF,

	LEMNI_TOKEN_TYPE_COUNT
} LemniTokenType;

typedef struct {
	LemniTokenType type;
	LemniStr text;
	LemniLocation loc;
} LemniToken;

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_TOKEN_H
