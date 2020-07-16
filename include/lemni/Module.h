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

#ifndef LEMNI_MODULE_H
#define LEMNI_MODULE_H 1

#include "Macros.h"

#include "lex.h"
#include "parse.h"
#include "typecheck.h"

/**
 * @defgroup Modules Module related types and functions
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Opaque type representing a module.
 */
LEMNI_OPAQUE_T(LemniModule);

/**
 * @brief Opaque type representing an executable environment.
 */
LEMNI_OPAQUE_T(LemniRuntime);

/**
 * @brief Create a module.
 * @note the returned module must be destroyed with \ref lemniDestroyModule .
 * @param types type set to use for typechecking
 * @returns handle to the newly created module
 */
LemniModule lemniCreateModule(LemniTypeSet types);

typedef enum LemniModuleResultTypeT{
	LEMNI_MODULE_RESULT_MODULE = 0,
	LEMNI_MODULE_RESULT_LEX,
	LEMNI_MODULE_RESULT_PARSE,
	LEMNI_MODULE_RESULT_TYPECHECK,
	LEMNI_MODULE_RESULT_COUNT
} LemniModuleResultType;

typedef struct LemniModuleResultT{
	LemniModuleResultType resType;
	union {
		LemniModule module;
		LemniLexError lexErr;
		LemniParseError parseErr;
		LemniTypecheckError typeErr;
	};
} LemniModuleResult;

/**
 * @brief Load a module from lemni source.
 * @param pathStr path to the source file
 * @returns handle to the newly create module
 */
LemniModuleResult lemniLoadModule(LemniTypeSet types, LemniStr pathStr);

typedef void(*LemniModuleTypecheckOkCB)(void *data, LemniTypedExpr *const exprs, const size_t numExprs);
typedef void(*LemniModuleTypecheckErrCB)(void *data, LemniTypecheckError err);

typedef struct LemniModuleTypecheckCBsT{
	void *data;
	LemniModuleTypecheckOkCB ok;
	LemniModuleTypecheckErrCB err;
} LemniModuleTypecheckCBs;

void lemniModuleTypecheck(LemniModule mod, const LemniExpr *const exprs, const size_t numExprs, LemniModuleTypecheckCBs cbs);

/**
 * @brief Destroy a module previously created with \ref lemniCreateModule .
 * @param mod handle of the module to destroy
 */
void lemniDestroyModule(LemniModule mod);

LemniTypeSet lemniModuleTypeSet(LemniModule mod);

size_t lemniModuleNumExprs(LemniModule mod);
LemniTypedExpr *lemniModuleExprs(LemniModule mod);

/**
 * @brief JIT compile a module.
 * @note the returned runtime must be destroyed with \ref lemniDestroyRuntime .
 * @param mod handle of the module to compile
 * @returns handle to a newly compiled runtime environment
 */
LemniRuntime lemniModuleJITCompile(LemniModule mod);

/**
 * @brief Destroy a runtime previously created with \ref lemniModuleJITCompile .
 * @param rt handle of the runtime to destroy
 */
void lemniDestroyRuntime(LemniRuntime rt);

typedef const struct LemniBoundParamsT *LemniBoundParams;

struct LemniRtValueT{
	LemniType type;
	union {
		uint64_t data;
		void *ptr;
	};
};

typedef struct LemniRtValueT LemniRtValue[1];

/**
 * @brief Type representing an executable lemni function.
 * To call the function, first an environment must be created with createEnv.
 * The created environment can be re-used for successive function calls, but is not thread-safe.
 * Before discarding the function the environment must be freed with destroyEnv.
 */
typedef struct LemniRtFnT{
	void*(*createEnv)(LemniRuntimeConst rt);
	void(*destroyEnv)(void *env);
	LemniBoundParams(*bindParams)(void *env, ...);
	void(*fn)(void *env, LemniRtValue res, LemniBoundParams args);
} LemniRtFn;

/**
 * @brief Destroy a set of parameters bound previously by \ref LemniRtFn::bindParams .
 * @param bound bound parameters
 */
void lemniDestroyBoundParams(LemniBoundParams bound);

/**
 * @brief Retrieve a function pointer from \p rt .
 * \p name is relative to the m
 * @param rt handle of the runtime to retrieve the function from
 * @param name name of the function to retrieve
 * @returns pointer to the function of ``NULL`` if no function was found
 */
LemniRtFn lemniRuntimeFn(LemniRuntimeConst rt, LemniStr name);

#ifdef __cplusplus
}

namespace lemni{
	class RuntimeFn{
	};

	class Runtime{
		public:
	};

	class Module{
		public:
			explicit Module(LemniTypeSet types) noexcept
				: m_handle(lemniCreateModule(types)){}

			Module(Module &&other) noexcept
				: m_handle(other.m_handle)
			{
				other.m_handle = nullptr;
			}

			~Module(){
				if(m_handle) lemniDestroyModule(m_handle);
			}

			static Module from(LemniModule handle) noexcept{
				return Module(handle);
			}

			Module &operator=(Module &&other) noexcept{
				if(m_handle) lemniDestroyModule(m_handle);

				m_handle = other.m_handle;
				other.m_handle = nullptr;

				return *this;
			}

			LemniModule handle() noexcept{ return m_handle; }
			LemniModuleConst handle() const noexcept{ return m_handle; }

			LemniTypeSet types() const noexcept{ return lemniModuleTypeSet(m_handle); }

			std::size_t numExprs() const noexcept{ return lemniModuleNumExprs(m_handle); }

			LemniTypedExpr *exprs() const noexcept{ return lemniModuleExprs(m_handle); }

		private:
			Module(LemniModule handle_) noexcept
				: m_handle(handle_){}

			LemniModule m_handle;
	};
}
#endif

/**
 * @}
 */

#endif // !LEMNI_MODULE_H
