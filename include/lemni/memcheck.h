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

#ifndef LEMNI_MEMCHECK_H
#define LEMNI_MEMCHECK_H 1

/**
 * @defgroup Regions Region-based memory management
 * @{
 */

#include "TypedExpr.h"
#include "Region.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief A type and memory checked expression */
typedef struct LemniMemCheckExprT{
	LemniTypedExpr expr; /** the typed expression */
	LemniMemorySize size; /** storage size required for value. may be { 0, 0 } */
} LemniMemCheckExpr;

typedef struct LemniMemCheckErrorT{
	LemniStr msg;
} LemniMemCheckError;

typedef struct LemniMemCheckResultT{
	bool hasError;
	union {
		LemniMemCheckError error;
		LemniMemCheckExpr res;
	};
} LemniMemCheckResult;

typedef struct LemniMemCheckStateT *LemniMemCheckState;
typedef const struct LemniMemCheckStateT *LemniMemCheckStateConst;

/**
 * @brief Create new memory checking state.
 * @param global region to use as a global pool. pass ``NULL`` to create new regions for all globals.
 * @returns
 */
LemniMemCheckState lemniCreateMemCheckState(LemniRegion global);

/**
 * @brief Destroy memory checking state.
 * @param state state to destroy
 */
void lemniDestroyMemCheckState(LemniMemCheckState state);

/**
 * @brief Memory check a typed expression
 * @param state state to modify
 * @param expr expression to memory check
 * @returns result of the operation
 */
LemniMemCheckResult lemniMemCheck(LemniMemCheckState state, LemniTypedExpr expr);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !LEMNI_MEMCHECK_H
