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

#ifndef LEMNI_LIB_TYPELIST_HPP
#define LEMNI_LIB_TYPELIST_HPP 1

#include <functional>

#include "lemni/Type.h"
#include "lemni/Interop.h"

namespace lemni::typelist{
	template<typename T>
	struct TypeFunctor{
		template<typename ... Args>
		static auto apply(Args &&... args) noexcept{
			return T::doApply(std::forward<Args>(args)...);
		}
	};

	template<typename ... Ts>
	struct TypeList: TypeFunctor<TypeList<Ts...>>{
		template<typename ... Args>
		static TypeList doApply(Args &&...) noexcept{ return {}; }
	};

	template<template<typename...> typename F, typename ... Ts>
	struct Partial: TypeFunctor<Partial<F, Ts...>>{
		template<typename ... Args>
		static Partial doApply(Args &&...) noexcept{ return {}; }
	};

	template<typename ... Ts>
	inline constexpr auto types = TypeList<Ts...>{};

	template<typename...> struct AppendF;
	template<typename...> struct CatF;
	template<typename...> struct AppF;
	template<typename...> struct TypeAppF;

	/**
	 * Append metafunction
	 */

	template<typename ... Ts, typename T>
	struct AppendF<TypeList<Ts...>, T>: TypeFunctor<AppendF<TypeList<Ts...>, T>>{
		using type = TypeList<Ts..., T>;

		template<typename ... Args>
		static auto doApply(Args &&... args) noexcept{ return type::apply(std::forward<Args>(args)...); }
	};

	template<typename ... Ts>
	using Append = typename AppendF<Ts...>::type;

	/**
	 * Concatenate metafunction
	 */

	template<typename ... Lhs, typename ... Rhs>
	struct CatF<TypeList<Lhs...>, TypeList<Rhs...>>: TypeFunctor<CatF<TypeList<Lhs...>, TypeList<Rhs...>>>{
		using type = TypeList<Lhs..., Rhs...>;

		template<typename ... Args>
		static auto doApply(Args &&... args) noexcept{ return type::apply(std::forward<Args>(args)...); }
	};

	template<typename ... Ts>
	using Cat = typename CatF<Ts...>::type;

	/**
	 * Metafunction application
	 */

	template<template<typename...> typename F, typename ... Ts, typename ... Args>
	struct AppF<Partial<F, Ts...>, Args...>: TypeFunctor<AppF<Partial<F, Ts...>, Args...>>{
		using type = typename F<Ts..., Args...>::type;

		template<typename ... UArgs>
		static auto doApply(UArgs &&... uargs) noexcept{ return type::apply(std::forward<UArgs>(uargs)...); }
	};

	template<template<typename...> typename F, typename ... Args>
	using App = typename AppF<Partial<F>, Args...>::type;

	/**
	 * Dynamic metafunction visitor
	 */

	template<template<typename...> typename F, typename ... Ts, typename ... Us>
	struct TypeAppF<Partial<F, Ts...>, TypeList<Us...>>: TypeFunctor<TypeAppF<Partial<F, Ts...>, TypeList<Us...>>>{
		template<typename ... Vs>
		using Call = F<Ts..., Vs..., Us...>;

		template<typename ... Args>
		static auto doApply(LemniType type, Args &&... args) noexcept{
			if(lemniTypeAsUnit(type)){
				return Call<LemniUnit>::apply(std::forward<Args>(args)...);
			}
			else if(lemniTypeAsBool(type)){
				return Call<LemniBool>::apply(std::forward<Args>(args)...);
			}
			else if(lemniTypeAsNat(type)){
				switch(lemniTypeNumBits(type)){
					case 16: return Call<LemniNat16>::apply(std::forward<Args>(args)...);
					case 32: return Call<LemniNat32>::apply(std::forward<Args>(args)...);
					case 64: return Call<LemniNat64>::apply(std::forward<Args>(args)...);
					default:{
						assert(!"non-system number of bits in natural type");
						break;
					}
				}
			}
			else if(lemniTypeAsInt(type)){
				switch(lemniTypeNumBits(type)){
					case 16: return Call<LemniInt16>::apply(std::forward<Args>(args)...);
					case 32: return Call<LemniInt32>::apply(std::forward<Args>(args)...);
					case 64: return Call<LemniInt64>::apply(std::forward<Args>(args)...);
					default:{
						assert(!"non-system number of bits in integer type");
						break;
					}
				}
			}
			else if(lemniTypeAsRatio(type)){
				switch(lemniTypeNumBits(type)){
					case 32: return Call<LemniRatio32>::apply(std::forward<Args>(args)...);
					case 64: return Call<LemniRatio64>::apply(std::forward<Args>(args)...);
					case 128: return Call<LemniRatio128>::apply(std::forward<Args>(args)...);
					default:{
						assert(!"non-system number of bits in rational type");
						break;
					}
				}
			}
			else if(lemniTypeAsReal(type)){
				switch(lemniTypeNumBits(type)){
					case 32: return Call<LemniReal32>::apply(std::forward<Args>(args)...);
					case 64: return Call<LemniReal64>::apply(std::forward<Args>(args)...);
					default:{
						assert(!"non-system number of bits in real type");
						break;
					}
				}
			}
			else if(lemniTypeAsStringUTF8(type)){
				return Call<LemniStrUTF8>::apply(std::forward<Args>(args)...);
			}
			else{
				assert(!"non-representable type");
			}
		}
	};

	template<typename T, typename ... Ts>
	constexpr auto append(TypeList<Ts...>) -> TypeList<Ts..., T>{ return {}; }

	template<typename ... Lhs, typename ... Rhs>
	constexpr auto cat(TypeList<Lhs...>, TypeList<Rhs...>) -> TypeList<Lhs..., Rhs...>{ return {}; }

	template<template<typename...> typename F, typename ... Ts, typename ... Args>
	constexpr auto apply(Partial<F, Ts...>, TypeList<Args...>) -> App<F, Ts..., Args...>{ return {}; }

	template<template<typename...> typename F, typename ... Ts, typename ... Us, typename ... Args>
	inline auto typeApply(Partial<F, Ts...>, TypeList<Us...>, LemniType type, Args &&... args){
		return TypeAppF<Partial<F, Ts...>, TypeList<Us...>>::apply(type, std::forward<Args>(args)...);
	}
}

#endif // !LEMNI_LIB_TYPELIST_HPP
