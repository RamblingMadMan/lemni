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
#include "compile.h"

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
LEMNI_OPAQUE_T(LemniModuleMap);

/**
 * @brief Opaque type representing an executable environment.
 */
LEMNI_OPAQUE_T(LemniRuntime);

/**
 * @brief Create a module.
 * @note the returned module must be destroyed with \ref lemniDestroyModule .
 * @param types type set to use for typechecking
 * @param id identifier of the module
 * @returns handle to the newly created module
 */
LemniModule lemniCreateModule(LemniModuleMap mods, const LemniStr id);

/**
 * @brief Destroy a module previously created with \ref lemniCreateModule .
 * @param mod handle of the module to destroy
 */
void lemniDestroyModule(LemniModule mod);

LemniStr lemniModuleId(LemniModuleConst mod);

LemniTypeSet lemniModuleTypeSet(LemniModuleConst mod);

LemniTypecheckStateConst lemniModuleTypecheckState(LemniModuleConst mod);

typedef enum LemniModuleResultTypeT{
	LEMNI_MODULE_RESULT_MODULE = 0,
	LEMNI_MODULE_LEX_ERROR,
	LEMNI_MODULE_PARSE_ERROR,
	LEMNI_MODULE_TYPECHECK_ERROR,
	LEMNI_MODULE_COMPILE_ERROR,
	LEMNI_MODULE_RESULT_COUNT
} LemniModuleResultType;

typedef struct LemniModuleResultT{
	LemniModuleResultType resType;
	union {
		LemniModule module;
		LemniLexError lexErr;
		LemniParseResultError parseErr;
		LemniTypecheckError typeErr;
		LemniCompileError compErr;
	};
} LemniModuleResult;

LemniTypedExtFnDeclExpr lemniModuleCreateExtFn(
	LemniModule mod, const LemniStr name, void *const ptr,
	const LemniType resultType,
	const LemniNat64 numParams,
	const LemniType *const paramTypes,
	const LemniStr *const paramNames
);

size_t lemniModuleNumExprs(LemniModule mod);
LemniTypedExpr *lemniModuleExprs(LemniModule mod);

/**
 * @brief JIT compile a module.
 * @note the returned runtime must be destroyed with \ref lemniDestroyRuntime .
 * @param mod handle of the module to compile
 * @returns result of the compilation \see lemniCompile
 */
LemniCompileResult lemniModuleJITCompile(LemniModule mod);

/**
 * @brief Create a new module map.
 * @param types typeset to use for loaded modules
 * @returns newly created module map
 */
LemniModuleMap lemniCreateModuleMap(LemniTypeSet types);

/**
 * @brief Destroy a module map previously created with \ref lemniCreateModuleMap .
 * @param mods module map to destroy
 */
void lemniDestroyModuleMap(LemniModuleMap mods);

/**
 * @brief Retrieve the typeset used for a module map.
 * @param mods module map to query
 * @returns resultant typeset
 */
LemniTypeSet lemniModuleMapTypes(LemniModuleMap mods);

/**
 * @brief Load a module from lemni source.
 * @param pathStr path to the source file
 * @returns handle to the newly create module
 */
LemniModuleResult lemniLoadModule(LemniModuleMap mods, const LemniStr id);

void lemniAliasModule(LemniModuleMap mods, const LemniStr id, const LemniStr alias);

void lemniRegisterModule(LemniModuleMap mods, LemniModule module);

#ifdef __cplusplus
}

namespace lemni{
	class Module{
		public:
			Module(LemniModuleMap mods, const LemniStr id) noexcept
				: m_handle(lemniCreateModule(mods, id)){}

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

			operator LemniModule() noexcept{ return m_handle; }
			operator LemniModuleConst() const noexcept{ return m_handle; }

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

	class ModuleMap{
		public:
			explicit ModuleMap(LemniTypeSet types) noexcept
				: m_handle(lemniCreateModuleMap(types)){}

			ModuleMap(ModuleMap &&other) noexcept
				: m_handle(other.m_handle)
			{
				other.m_handle = nullptr;
			}

			~ModuleMap(){
				if(m_handle) lemniDestroyModuleMap(m_handle);
			}

			static ModuleMap from(LemniModuleMap handle_) noexcept{
				return ModuleMap(handle_);
			}

			operator LemniModuleMap() noexcept{ return m_handle; }
			operator LemniModuleMapConst() const noexcept{ return m_handle; }

			ModuleMap &operator=(ModuleMap &&other) noexcept{
				if(m_handle) lemniDestroyModuleMap(m_handle);

				m_handle = other.m_handle;
				other.m_handle = nullptr;

				return *this;
			}

			void register_(LemniModule module) noexcept{
				lemniRegisterModule(m_handle, module);
			}

			void alias(const std::string_view id, const std::string_view alias) noexcept{
				lemniAliasModule(m_handle, lemni::fromStdStrView(id), lemni::fromStdStrView(alias));
			}

		private:
			explicit ModuleMap(LemniModuleMap handle_) noexcept
				: m_handle(handle_){}

			LemniModuleMap m_handle;
	};
}
#endif

/**
 * @}
 */

#endif // !LEMNI_MODULE_H
