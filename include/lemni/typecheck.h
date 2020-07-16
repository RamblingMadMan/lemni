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

#ifndef LEMNI_TYPECHECK_H
#define LEMNI_TYPECHECK_H 1

#include "Expr.h"
#include "TypedExpr.h"

/**
 * @defgroup Typechecking Typechecking related types and functions.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

//! Opaque type representing typechecking state.
typedef struct LemniTypecheckStateT *LemniTypecheckState;

typedef const struct LemniTypecheckStateT *LemniTypecheckStateConst;

//! Type representing a typechecking error
typedef struct {
	LemniLocation loc;
	LemniStr msg;
} LemniTypecheckError;

//! Type representing the result of a typechecking operation.
typedef struct {
	bool hasError;
	union {
		LemniTypecheckError error;
		LemniTypedExpr expr;
	};
} LemniTypecheckResult;

/**
 * @brief Create new state for typechecking functions.
 * @note the returned state must be destroyed with \ref lemniDestroyTypecheckState .
 * @warning the pointer \p exprs must stay valid for the life of the returned state.
 * @param types typeset to get types from
 * @param exprs pointer to array of expressions to typecheck
 * @param numExprs number of expressions in \p exprs array
 * @returns newly created typechecking state
 */
LemniTypecheckState lemniCreateTypecheckState(LemniTypeSet types);

/**
 * @brief Destroy state previously created with \ref lemniCreateTypecheckState .
 * @warning ``NULL`` must not be passed to this function.
 * @param state the state to destroy
 */
void lemniDestroyTypecheckState(LemniTypecheckState state);

LemniTypedExtFnDeclExpr lemniCreateTypedExtFn(
	LemniTypecheckState state, const LemniStr name, void *const ptr,
	const LemniType resultType,
	const LemniNat64 numParams,
	const LemniType *const paramTypes,
	const LemniStr *const paramNames
);

/**
 * @brief Typecheck a single expression from \p state .
 * @param state the state to modify
 * @returns the result of the typechecking operation
 */
LemniTypecheckResult lemniTypecheck(LemniTypecheckState state, LemniExpr expr);

/**
 * @brief Check the result type of a unary op on a type
 * @param types typeset to get the type from
 * @param value value type of the unary op
 * @param op the operator to check against
 * @return the result of the type operation or NULL if the operation is undefined
 */
LemniType lemniUnaryOpResultType(LemniTypeSet types, LemniType value, LemniUnaryOp op);

/**
 * @brief Check the result type of a binary op between two types
 * @param types typeset to get types from
 * @param lhs left hand side type of the operation
 * @param rhs right hand side type of the operation
 * @param op the operator to check against
 * @returns the result type of the operation or NULL if the operation is undefined
 */
LemniType lemniBinaryOpResultType(LemniTypeSet types, LemniType lhs, LemniType rhs, LemniBinaryOp op);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <variant>
#include <vector>

namespace lemni{
	using TypedExpr = LemniTypedExpr;
	using TypecheckError = LemniTypecheckError;

	class TypecheckState{
		public:
			explicit TypecheckState(LemniTypeSet types)
				: m_state(lemniCreateTypecheckState(types)){}

			TypecheckState(TypecheckState &&other) noexcept
				: m_state(other.m_state)
			{
				other.m_state = nullptr;
			}

			TypecheckState(const TypecheckState&) = delete;

			~TypecheckState(){ if(m_state) lemniDestroyTypecheckState(m_state); }

			TypecheckState &operator=(TypecheckState &&other) noexcept{
				m_state = other.m_state;
				other.m_state = nullptr;
				return *this;
			}

			TypecheckState &operator=(const TypecheckState&) = delete;

			operator LemniTypecheckState() noexcept{ return m_state; }
			operator LemniTypecheckStateConst() const noexcept{ return m_state; }

		private:
			LemniTypecheckState m_state;

			friend inline std::variant<TypedExpr, TypecheckError> typecheck(TypecheckState &state) noexcept;
	};

	inline std::variant<TypedExpr, TypecheckError> typecheck(LemniTypecheckState state, LemniExpr expr) noexcept{
		auto res = lemniTypecheck(state, expr);
		if(res.hasError)
			return res.error;
		else
			return res.expr;
	}

	inline std::variant<std::vector<TypedExpr>, TypecheckError> typecheckAll(LemniTypecheckState state, const std::vector<Expr> &exprs){
		std::vector<TypedExpr> typedExprs;
		typedExprs.reserve(exprs.size());

		for(auto expr : exprs){
			auto res = lemniTypecheck(state, expr);
			if(res.hasError)
				return res.error;
			else{
				auto typed = res.expr;
				if(!typed) break;
				else typedExprs.emplace_back(typed);
			}
		}

		return std::move(typedExprs);
	}

	inline std::pair<TypecheckState, std::variant<std::vector<TypedExpr>, TypecheckError>> typecheckAll(LemniTypeSet types, const std::vector<Expr> &exprs){
		auto state = TypecheckState(types);

		auto res = typecheckAll(state, exprs);

		return std::make_pair(std::move(state), std::move(res));
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_TYPECHECK_H
