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

#ifndef LEMNI_SCOPE_H
#define LEMNI_SCOPE_H 1

#include "Macros.h"
#include "TypedExpr.h"

/**
 * @defgroup Scope Lexical scope
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdbool>

typedef struct LemniScopeT *LemniScope;
typedef const struct LemniScopeT *LemniScopeConst;

typedef struct LemniScopeResultT{
	bool hasError;
} LemniScopeResult;

LemniScope lemniCreateScope(LemniScopeConst parent);

void lemniDestroyScope(LemniScope s);

LemniTypedLValueExpr lemniScopeFind(LemniScopeConst s, LemniStr name);

bool lemniScopeSet(LemniScope s, LemniTypedLValueExpr expr);

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
namespace lemni{
	class Scope{
		public:
			explicit Scope(LemniScopeConst parent = nullptr) noexcept
				: m_handle(lemniCreateScope(parent)){}

			explicit Scope(Scope *parent) noexcept
				: m_handle(lemniCreateScope(parent->handle())){}

			Scope(Scope &&other) noexcept
				: m_handle(other.m_handle)
			{
				other.m_handle = nullptr;
			}

			~Scope(){
				if(m_handle)
					lemniDestroyScope(m_handle);
			}

			operator LemniScope() noexcept{ return m_handle; }
			operator LemniScopeConst() const noexcept{ return m_handle; }

			LemniScope handle() noexcept{ return m_handle; }
			LemniScopeConst handle() const noexcept{ return m_handle; }

			auto find(std::string_view name) const noexcept{
				return lemniScopeFind(m_handle, lemni::fromStdStrView(name));
			}

			bool set(LemniTypedLValueExpr expr) noexcept{
				return lemniScopeSet(m_handle, expr);
			}

		private:
			LemniScope m_handle;
	};
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_SCOPE_H
