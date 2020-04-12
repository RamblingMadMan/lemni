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

#ifndef LEMNI_EVAL_H
#define LEMNI_EVAL_H 1

#include "TypedExpr.h"
#include "Value.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to evaluation state
 */
typedef struct LemniEvalStateT *LemniEvalState;

/**
 * @brief Type representing an evaluation error.
 */
typedef struct {
	LemniStr msg;
} LemniEvalError;

/**
 * @brief Type representing the result of an evaluation.
 */
typedef struct {
	bool hasError;
	union {
		LemniEvalError error;
		LemniValue value;
	};
} LemniEvalResult;

/**
 * @brief Create state for evaluating typed expressions.
 * @note the returned state must be destroyed with \ref lemniDestroyEvalState .
 * @return The newly created state
 */
LemniEvalState lemniCreateEvalState(void);

/**
 * @brief Destroy state previously created with \ref lemniCreateEvalState .
 * @warning \p state must be a valid pointer.
 * @param state state to destroy
 */
void lemniDestroyEvalState(LemniEvalState state);

/**
 * @brief Evaluate a typed expression.
 * @warning the returned value must be destroyed with \ref lemniDestroyValue .
 * @param state state to modify
 * @param expr expression to evaluate
 * @return The result of the operation
 */
LemniEvalResult lemniEval(LemniEvalState state, LemniTypedExpr expr);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <variant>

namespace lemni{
	class EvalState{
		public:
			EvalState(): m_state(lemniCreateEvalState()){}

			EvalState(EvalState &&other) noexcept: m_state(other.m_state){ other.m_state = nullptr; }

			EvalState(const EvalState&) = delete;

			~EvalState(){ if(m_state) lemniDestroyEvalState(m_state); }

			EvalState &operator=(EvalState &&other) noexcept{
				m_state = other.m_state;
				other.m_state = nullptr;
				return *this;
			}

			EvalState &operator=(const EvalState&) = delete;

			LemniEvalState handle() noexcept{ return m_state; }

		private:
			LemniEvalState m_state;
	};

	inline std::variant<Value, LemniEvalError> eval(EvalState &state, TypedExpr expr) noexcept{
		auto result = lemniEval(state.handle(), expr);
		if(result.hasError){
			return result.error;
		}
		else{
			return Value::from(result.value);
		}
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_EVAL_H
