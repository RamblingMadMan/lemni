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

#ifndef LEMNI_COMPILE_H
#define LEMNI_COMPILE_H 1

#include "TypedExpr.h"
#include "memcheck.h"

/**
 * @defgroup Compile Types and functions related to expression compilation.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to compilation state.
 */
typedef struct LemniCompileStateT *LemniCompileState;

typedef struct LemniObjectT *LemniObject;

typedef struct LemniCompileErrorT{
	LemniStr msg;
	LemniLocation loc;
} LemniCompileError;

typedef struct LemniCompileResultT{
	bool hasError;
	union {
		LemniObject object;
		LemniCompileError error;
	};
} LemniCompileResult;

/**
 * @brief Create new compile state.
 * @note the returned handle must be destroyed with \ref lemniDestroyCompileState .
 * @returns handle to newly created state
 */
LemniCompileState lemniCreateCompileState(LemniCompileState parent);

/**
 * @brief Destroy state previously created with \ref lemniCreateCompileState .
 * @param state handle to state to destroy
 */
void lemniDestroyCompileState(LemniCompileState state);

/**
 * @brief Compile an array of typed expressions in order.
 * @param state handle to compile state
 * @param exprs pointer to typed expressions
 * @param numExprs number of expressions in \p exprs .
 * @returns result of the compilation
 */
LemniCompileResult lemniCompile(LemniCompileState state, LemniTypedExpr *const exprs, const LemniNat64 numExprs);

/**
 * @brief Destroy an object previously created by \ref lemniCompile .
 * @param obj handle to the object to destroy
 */
void lemniDestroyObject(LemniObject obj);

typedef void(*LemniFn)();

/**
 * @brief Retrieve a function from a compiled object.
 * @param obj handle of the object to query
 * @param mangledName mangled name of the function
 * @returns the resolved function or ``NULL``
 */
LemniFn lemniObjectFunction(LemniObject obj, const LemniStr mangledName);

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
namespace lemni{
	class CompileState{
		public:
			explicit CompileState(LemniCompileState parent_ = nullptr) noexcept
				: m_state(lemniCreateCompileState(parent_)){}

			~CompileState(){ if(m_state) lemniDestroyCompileState(m_state); }

			CompileState &operator=(CompileState &&other) noexcept{
				if(m_state) lemniDestroyCompileState(m_state);
				m_state = other.m_state;
				other.m_state = nullptr;
				return *this;
			}

			CompileState &operator=(const CompileState&) = delete;

			operator LemniCompileState() noexcept{ return m_state; }

			LemniCompileState handle() noexcept{ return m_state; }

		private:
			LemniCompileState m_state;
	};
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_COMPILE_H
