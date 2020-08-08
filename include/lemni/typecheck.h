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
#include "Scope.h"

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

typedef struct LemniModuleT *LemniModule;

typedef struct LemniModuleMapT *LemniModuleMap;

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
 * @param mods module map to retrieve modules and types from
 * @param module module that the typecheck state belongs to
 * @returns newly created typechecking state
 */
LemniTypecheckState lemniCreateTypecheckState(LemniModuleMap mods);

/**
 * @brief Destroy state previously created with \ref lemniCreateTypecheckState .
 * @warning ``NULL`` must not be passed to this function.
 * @param state the state to destroy
 */
void lemniDestroyTypecheckState(LemniTypecheckState state);

LemniModuleMap lemniTypecheckStateModuleMap(LemniTypecheckState state);

LemniScope lemniTypecheckStateScope(LemniTypecheckStateConst state);

LemniTypedPlaceholderExpr lemniTypecheckPlaceholder(LemniTypecheckState state);

/**
 * @brief Evaluate \p expr optionally applied to \p args .
 * The passed expression may be partially evaluated i.e. \p numArgs may be less than the number of params for \p expr .
 * To skip an argument, ``NULL`` or a typed placeholder may be passed in the \p args array.
 * If \p numArgs is greater than the number of params in \p expr , ``NULL`` will be returned.
 * Evaluating a non-function expressions expects ``0`` for \p numArgs and won't check \p args .
 * @param state typechecking state to use
 * @param expr expression to evaluate
 * @param numArgs number of elements \p args points to
 * @param args pointer to the arguments
 * @returns result of the evaluation
 */
LemniTypecheckResult lemniTypecheckEval(LemniTypecheckState state, LemniTypedExpr expr, const LemniNat64 numArgs, LemniTypedExpr *const args);

/**
 * @brief Create a module expression for typechecking to resolve from.
 * @param state typechecking state to modify
 * @param name alias for the new module
 * @param id identifier for loading the module (relative/absolute path or registered name)
 * @returns newly created module expression
 */
LemniTypedModuleExpr lemniCreateTypedModule(LemniTypecheckState state, const LemniStr alias, LemniModule module);

/**
 * @brief Create an external function expression for typechecking to resolve.
 * @param state typechecking state to modify
 * @param name valid lemni function identifier for the function
 * @param ptr pointer to the function
 * @param resultType result type of a function call
 * @param numParams number of function parameters
 * @param paramTypes pointer to array of \p numParams \ref LemniType s
 * @param paramNames pointer to array of \p numParams \ref LemniStr s
 * @returns newly created external function expression
 */
LemniTypedExtFnDeclExpr lemniCreateTypedExtFn(
	LemniTypecheckState state, const LemniStr name, void *const ptr,
	const LemniType resultType,
	const LemniNat64 numParams,
	const LemniType *const paramTypes,
	const LemniStr *const paramNames
);

/**
 * @brief Typecheck a single expression from \p state .
 * @param state typechecking state to modify
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
			explicit TypecheckState(LemniModuleMap mods)
				: m_state(lemniCreateTypecheckState(mods)){}

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

	inline std::pair<TypecheckState, std::variant<std::vector<TypedExpr>, TypecheckError>> typecheckAll(LemniModuleMap mods, const std::vector<Expr> &exprs){
		auto state = TypecheckState(mods);

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
