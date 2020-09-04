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

#ifndef LEMNI_INTEROP_H
#define LEMNI_INTEROP_H 1

/**
 * @defgroup Interop Types and functions for C interoperability with Lemni
 * These types should be used in any interfaces directly used by lemni objects.
 * @{
 */

#include "Str.h"

#if defined(__GNUC__)
#define BEGIN_PACKED() _Pragma("pack(push, 1)")
#define END_PACKED() _Pragma("pack(pop)")
#elif defined(_MSC_VER)
#define BEGIN_PACKED() __pragma(pack(push, 1))
#define END_PACKED() __pragma(pack(pop))
#else
#error "No packing compiler directive found"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

typedef const struct LemniUnitT *const LemniUnit;

BEGIN_PACKED();
typedef enum LemniBoolValueT{
	LEMNI_FALSE = 0,
	LEMNI_TRUE = 1
} LemniBoolValue;
END_PACKED();

typedef uint8_t LemniBool;

typedef uint16_t LemniNat16;
typedef uint32_t LemniNat32;
typedef uint64_t LemniNat64;

typedef int16_t LemniInt16;
typedef int32_t LemniInt32;
typedef int64_t LemniInt64;

/**
 * @defgroup Ratios Ratio literal types
 * @{
 */

BEGIN_PACKED();

typedef struct alignas(4) LemniRatio32T{
	LemniInt16 num;
	LemniNat16 den;
} LemniRatio32;

typedef struct alignas(8) LemniRatio64T{
	LemniInt32 num;
	LemniNat32 den;
} LemniRatio64;

typedef struct alignas(16) LemniRatio128T{
	LemniInt64 num;
	LemniNat64 den;
} LemniRatio128;

END_PACKED();

LemniRatio32 lemniSimplifyRatio32(const LemniRatio32 q32);
LemniRatio64 lemniSimplifyRatio64(const LemniRatio64 q64);
LemniRatio128 lemniSimplifyRatio128(const LemniRatio128 q128);

/**
 * @}
 */

/**
 * @defgroup Reals Real literal types
 * @{
 */

#ifndef __STDC_IEC_559__
#warning "Could not determine if floating point types are IEEE-754 compliant"
#endif

typedef float LemniReal32;
typedef double LemniReal64;

/**
 * @}
 */

typedef struct LemniStrUTF8T{ const char *ptr; LemniNat64 len; } LemniStrUTF8;
typedef struct LemniStrASCIIT{ const char *ptr; LemniNat64 len; } LemniStrASCII;

inline LemniStr lemniUTF8AsStr(LemniStrUTF8 utf8){ return { .ptr = utf8.ptr, .len = utf8.len }; }
inline LemniStrUTF8 lemniASCIIAsUTF8(LemniStrASCII ascii){ return { .ptr = ascii.ptr, .len = ascii.len }; }

/**
 * @defgroup Objects Object data for C interop
 * @{
 */

/**
 * @brief Lemni object data
 * Any data with an object header `o` returned from lemni must be destroyed by `o.dtor(o.env)`,
 * or a call to `lemniDestroyObject(o)`.
 */
typedef struct LemniObjectDataT{
	void(*dtor)(void*);
	void *env;
} LemniObjectData;

static inline void lemniDestroyObjectData(LemniObjectData *obj){
	if(obj->dtor) obj->dtor(obj->env);
}

/**
 * @}
 */

/**
 * @brief Generic functor object type
 */
typedef struct alignas(16) LemniFunctorT{
	LemniObjectData obj;
	void(*fptr)();
} LemniFunctor;

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
#include <type_traits>

namespace lemni{
	class AInt;
	class ARatio;
	class AReal;
}

namespace lemni::detail{
	template<typename T>
	struct is_natural: std::integral_constant<bool,
			std::is_same_v<T, LemniNat16> ||
			std::is_same_v<T, LemniNat32> ||
			std::is_same_v<T, LemniNat64>
		>{};

	template<typename T>
	struct is_integer: std::integral_constant<bool,
			std::is_same_v<T, LemniInt16> ||
			std::is_same_v<T, LemniInt32> ||
			std::is_same_v<T, LemniInt64> ||
			std::is_same_v<T, AInt>
		>{};

	template<typename T>
	struct is_ratio: std::integral_constant<bool,
			std::is_same_v<T, LemniRatio32> ||
			std::is_same_v<T, LemniRatio64> ||
			std::is_same_v<T, LemniRatio128> ||
			std::is_same_v<T, ARatio>
		>{};

	template<typename T>
	struct is_real: std::integral_constant<bool,
			std::is_same_v<T, LemniReal32> ||
			std::is_same_v<T, LemniReal64> ||
			std::is_same_v<T, AReal>
		>{};

	template<typename Ratio>
	struct ratio_traits{
		static_assert(is_ratio<Ratio>::value);
		static constexpr std::size_t num_bits = sizeof(Ratio) * CHAR_BIT;
		using num_type = decltype(Ratio::num);
		using den_type = decltype(Ratio::den);
	};

	template<typename T>
	constexpr auto is_natural_v = is_natural<T>::value;

	template<typename T>
	constexpr auto is_integer_v = is_integer<T>::value;

	template<typename T>
	constexpr auto is_ratio_v = is_ratio<T>::value;

	template<typename T>
	constexpr auto is_real_v = is_real<T>::value;

	template<typename> struct promoted;

	template<typename T>
	using promoted_t = typename promoted<T>::type;

	template<> struct promoted<LemniNat16>{ using type = LemniNat32; };
	template<> struct promoted<LemniNat32>{ using type = LemniNat64; };
	template<> struct promoted<LemniNat64>{ using type = lemni::AInt; };

	template<> struct promoted<LemniInt16>{ using type = LemniInt32; };
	template<> struct promoted<LemniInt32>{ using type = LemniInt64; };
	template<> struct promoted<LemniInt64>{ using type = lemni::AInt; };

	template<> struct promoted<LemniRatio32>{ using type = LemniRatio64; };
	template<> struct promoted<LemniRatio64>{ using type = LemniRatio128; };
	template<> struct promoted<LemniRatio128>{ using type = lemni::ARatio; };

	template<> struct promoted<LemniReal32>{ using type = LemniReal64; };
	template<> struct promoted<LemniReal64>{ using type = lemni::AReal; };

	template<std::size_t N> struct nat;
	template<> struct nat<16>{ using type = LemniNat16; };
	template<> struct nat<32>{ using type = LemniNat32; };
	template<> struct nat<64>{ using type = LemniNat64; };

	template<std::size_t N>
	using nat_t = typename nat<N>::type;

	template<std::size_t N> struct int_;
	template<> struct int_<16>{ using type = LemniInt16; };
	template<> struct int_<32>{ using type = LemniInt32; };
	template<> struct int_<64>{ using type = LemniInt64; };

	template<std::size_t N>
	using int_t = typename int_<N>::type;

	template<std::size_t N> struct ratio;
	template<> struct ratio<32>{ using type = LemniRatio32; };
	template<> struct ratio<64>{ using type = LemniRatio64; };
	template<> struct ratio<128>{ using type = LemniRatio128; };

	template<std::size_t N>
	using ratio_t = typename ratio<N>::type;

	template<std::size_t N> struct real;
	template<> struct real<32>{ using type = float; };
	template<> struct real<64>{ using type = double; };

	template<std::size_t N>
	using real_t = typename real<N>::type;
}

namespace lemni::interop{
	namespace detail{
		using namespace lemni::detail;

		template<typename Lhs, typename Rhs>
		constexpr inline auto addRatio(const Lhs &lhs, const Rhs &rhs){
			constexpr auto LhsN = ratio_traits<Lhs>::num_bits;
			constexpr auto RhsN = ratio_traits<Rhs>::num_bits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename promoted<GreaterQ>::type;
			using NumType = typename ratio_traits<PromotedQ>::num_type;
			using DenType = typename ratio_traits<PromotedQ>::den_type;
			return PromotedQ{
				(NumType(lhs.num) * rhs.den) + (NumType(rhs.num) * lhs.den),
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
		constexpr inline auto subRatio(const Lhs &lhs, const Rhs &rhs){
			constexpr auto LhsN = ratio_traits<Lhs>::num_bits;
			constexpr auto RhsN = ratio_traits<Rhs>::num_bits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename promoted<GreaterQ>::type;
			using NumType = typename ratio_traits<PromotedQ>::num_type;
			using DenType = typename ratio_traits<PromotedQ>::den_type;
			return PromotedQ{
				(NumType(lhs.num) * rhs.den) - (NumType(rhs.num) * lhs.den),
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
		constexpr inline auto mulRatio(const Lhs &lhs, const Rhs &rhs){
			constexpr auto LhsN = ratio_traits<Lhs>::num_bits;
			constexpr auto RhsN = ratio_traits<Rhs>::num_bits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename promoted<GreaterQ>::type;
			using NumType = typename ratio_traits<PromotedQ>::num_type;
			using DenType = typename ratio_traits<PromotedQ>::den_type;
			return PromotedQ{
				NumType(lhs.num) * rhs.num,
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
		constexpr inline auto divRatio(const Lhs &lhs, const Rhs &rhs){
			constexpr auto LhsN = ratio_traits<Lhs>::num_bits;
			constexpr auto RhsN = ratio_traits<Rhs>::num_bits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename promoted<GreaterQ>::type;
			using NumType = typename ratio_traits<PromotedQ>::num_type;
			using DenType = typename ratio_traits<PromotedQ>::den_type;

			auto newNum = NumType(lhs.num) * rhs.den;
			auto newDen = DenType(lhs.den) * rhs.num;

			if(newDen < 0){
				newDen *= -1;
				newNum *= -1;
			}

			return PromotedQ{newNum, newDen};
		}

		template<typename...>
		struct FnFunctorTypeT;

		template<typename Ret, typename ... Params>
		struct FnFunctorTypeT<Ret(Params...)>{
			using type = Ret(*)(const void*, Params...);
		};

		template<typename FnType>
		using FnFunctorType = typename FnFunctorTypeT<FnType>::type;
	}

	template<typename FnType, typename ... Args>
	std::result_of_t<FnType> callFunctor(const LemniFunctor &f, Args &&... args){
		return reinterpret_cast<detail::FnFunctorType<FnType>>(f.fptr)(f.obj.env, std::forward<Args>(args)...);
	}

	using Unit = LemniUnit;

	inline constexpr Unit unit = nullptr;

	using Bool = LemniBool;

	using Nat16 = LemniNat16;
	using Nat32 = LemniNat32;
	using Nat64 = LemniNat64;

	using Int16 = LemniInt16;
	using Int32 = LemniInt32;
	using Int64 = LemniInt64;

	template<size_t N>
	using Ratio = detail::ratio_t<N>;

	using Ratio32 = Ratio<32>;
	using Ratio64 = Ratio<64>;
	using Ratio128 = Ratio<128>;

	template<typename T>
	constexpr auto IsRatio = detail::is_ratio_v<T>;

	template<typename Q>
	using RatioNumType = typename detail::ratio_traits<Q>::num_type;

	template<typename Q>
	using RatioDenType = typename detail::ratio_traits<Q>::den_type;

	template<size_t N>
	using Real = detail::real_t<N>;

	using Real32 = Real<32>;
	using Real64 = Real<64>;

	using Str = LemniStr;

	template<typename...>
	class Functor;

	template<typename Ret, typename ... Params>
	class Functor<Ret(Params...)>{
		public:
			explicit Functor(Ret(*fptr)(Params...)) noexcept{
				functor.obj = { .dtor = [](auto...){}, .env = nullptr };
				functor.fptr = fptr;
			}

			template<typename Env>
			Functor(Env env, Ret(*fptr)(const Env*, Params...)){
				functor.obj = {
					.dtor = [](void *ptr){
						auto envptr = reinterpret_cast<Env*>(ptr);
						delete envptr;
					},
					.env = new Env(std::move(env))
				};

				functor.fptr = reinterpret_cast<void(*)()>(fptr);
			}

			Functor(Functor &&other) noexcept{
				functor.obj = other.functor.obj;
				functor.fptr = other.functor.fptr;

				other.functor.obj.dtor = [](auto...){};
				other.functor.obj.env = nullptr;
				other.functor.fptr = [](auto...){};
			}

			~Functor(){
				functor.obj.dtor(functor.obj.env);
			}

			Functor &operator=(Functor &&other) noexcept{
				functor.obj.dtor(functor.obj.env);

				functor.obj = other.functor.obj;
				functor.fptr = other.functor.fptr;

				other.functor.obj.dtor = [](auto...){};
				other.functor.obj.env = nullptr;
				other.functor.fptr = [](auto...){};

				return *this;
			}

			template<typename ... Args>
			Ret call(Args &&... args) const{
				return reinterpret_cast<Ret(*)(const void*, Params...)>(functor.fptr)(functor.obj.env, std::forward<Args>(args)...);
			}

			template<typename ... Args>
			Ret operator()(Args &&... args) const{
				return call(std::forward<Args>(args)...);
			}

		private:
			LemniFunctor functor;
	};

	inline Ratio32 simplifyRatio(const Ratio32 q32) noexcept{ return lemniSimplifyRatio32(q32); }
	inline Ratio64 simplifyRatio(const Ratio64 q64) noexcept{ return lemniSimplifyRatio64(q64); }
	inline Ratio128 simplifyRatio(const Ratio128 q128) noexcept{ return lemniSimplifyRatio128(q128); }
}

/**
 * @defgroup RatioOps Operators for working with rational numbers
 * @{
 */

template<typename Lhs, typename Rhs, std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, int> = 0>
inline auto operator +(const Lhs &lhs, const Rhs &rhs) noexcept{
	return lemni::interop::detail::addRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs, std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, int> = 0>
inline auto operator -(const Lhs &lhs, const Rhs &rhs) noexcept{
	return lemni::interop::detail::subRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs, std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, int> = 0>
inline auto operator *(const Lhs &lhs, const Rhs &rhs) noexcept{
	return lemni::interop::detail::mulRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs, std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, int> = 0>
inline auto operator /(const Lhs &lhs, const Rhs &rhs) noexcept{
	return lemni::interop::detail::divRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator <(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot < rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem < rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator >(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot > rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem > rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator <=(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot < rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem <= rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator >=(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot > rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem >= rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator ==(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem == rhsDiv.rem);
}

template<typename Lhs, typename Rhs>
inline std::enable_if_t<lemni::detail::is_ratio_v<Lhs> && lemni::detail::is_ratio_v<Rhs>, bool> operator !=(const Lhs &lhs, const Rhs &rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot != rhsDiv.quot) || (lhsDiv.rem != rhsDiv.rem);
}

/**
 * @}
 */

#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_INTEROP_H
