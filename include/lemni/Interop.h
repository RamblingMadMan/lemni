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
 * @{
 */

#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

typedef const struct LemniUnitT *const LemniUnit;

typedef enum LemniBoolT : uint8_t{
	LEMNI_FALSE = 0,
	LEMNI_TRUE = 1
} LemniBool;

typedef uint16_t LemniNat16;
typedef uint32_t LemniNat32;
typedef uint64_t LemniNat64;

typedef int16_t LemniInt16;
typedef int32_t LemniInt32;
typedef int64_t LemniInt64;

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

#ifndef __STDC_IEC_559__
#error "Floating point types must be IEEE-754 compliant"
#else
typedef float LemniReal32;
typedef double LemniReal64;
#endif

typedef LemniStr LemniStrUTF8;
struct LemniStrASCII{ size_t len; const char *ptr; };

/**
 * @brief Generic functor type
 *
 * If a functor ``f`` is retrieved from an external source then
 * the user must call ``f.dtor(f.env)`` once finished.
 *
 */
typedef struct alignas(16) LemniFunctorT{
	void(*dtor)(void*);
	void(*fptr)();
	void *env;
} LemniFunctor;

LemniRatio32 lemniSimplifyRatio32(const LemniRatio32 q32);
LemniRatio64 lemniSimplifyRatio64(const LemniRatio64 q64);
LemniRatio128 lemniSimplifyRatio128(const LemniRatio128 q128);

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
#include <type_traits>

namespace lemni{
	class AInt;
	class ARatio;

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
			std::is_same_v<T, LemniInt64>
		>{};

	template<typename T>
	struct is_ratio: std::integral_constant<bool,
			std::is_same_v<T, LemniRatio32> ||
			std::is_same_v<T, LemniRatio64> ||
			std::is_same_v<T, LemniRatio128>
		>{};

	template<typename T>
	struct is_real: std::integral_constant<bool,
			std::is_same_v<T, LemniReal32> ||
			std::is_same_v<T, LemniReal64>
		>{};

	template<typename T>
	constexpr auto is_natural_v = is_natural<T>::value;

	template<typename T>
	constexpr auto is_integer_v = is_integer<T>::value;

	template<typename T>
	constexpr auto is_ratio_v = is_ratio<T>::value;

	template<typename T>
	constexpr auto is_real_v = is_real<T>::value;
}

namespace lemni::interop{
	namespace detail{
		template<size_t N> struct RealT;

		template<> struct RealT<32>{ using Type = LemniReal32; };
		template<> struct RealT<64>{ using Type = LemniReal64; };

		template<size_t N> struct RatioT;

		template<> struct RatioT<32>{ using Type = LemniRatio32; };
		template<> struct RatioT<64>{ using Type = LemniRatio64; };
		template<> struct RatioT<128>{ using Type = LemniRatio128; };

		template<typename> struct RatioTraits;

		template<> struct RatioTraits<LemniRatio32>{
			static constexpr auto numBits = 32;
			using NumType = LemniInt16;
			using DenType = LemniNat16;
		};

		template<> struct RatioTraits<LemniRatio64>{
			static constexpr auto numBits = 64;
			using NumType = LemniInt32;
			using DenType = LemniNat32;
		};

		template<> struct RatioTraits<LemniRatio128>{
			static constexpr auto numBits = 128;
			using NumType = LemniInt64;
			using DenType = LemniNat64;
		};

		template<> struct RatioTraits<lemni::ARatio>{
			static constexpr auto numBits = SIZE_MAX;
			using NumType = lemni::AInt;
			using DenType = lemni::AInt;
		};

		template<typename> struct IsIntT: std::false_type{};
		template<> struct IsIntT<LemniInt16>: std::true_type{};
		template<> struct IsIntT<LemniInt32>: std::true_type{};
		template<> struct IsIntT<LemniInt64>: std::true_type{};
		template<> struct IsIntT<lemni::AInt>: std::true_type{};

		template<typename> struct IsRatioT: std::false_type{};
		template<> struct IsRatioT<LemniRatio32>: std::true_type{};
		template<> struct IsRatioT<LemniRatio64>: std::true_type{};
		template<> struct IsRatioT<LemniRatio128>: std::true_type{};
		template<> struct IsRatioT<lemni::ARatio>: std::true_type{};

		template<typename> struct PromotedT;

		template<> struct PromotedT<LemniRatio32>{ using Type = LemniRatio64; };
		template<> struct PromotedT<LemniRatio64>{ using Type = LemniRatio128; };
		template<> struct PromotedT<LemniRatio128>{ using Type = lemni::ARatio; };

		template<typename T>
		concept Integral = IsIntT<T>::value;

		template<typename T>
		concept Rational = IsRatioT<T>::value;

		template<Rational Lhs, Rational Rhs>
			//requires Rational<Lhs> && Rational<Rhs>
		constexpr inline auto addRatio(const Lhs lhs, const Rhs rhs){
			constexpr auto LhsN = RatioTraits<Lhs>::numBits;
			constexpr auto RhsN = RatioTraits<Rhs>::numBits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename PromotedT<GreaterQ>::Type;
			using NumType = typename RatioTraits<PromotedQ>::NumType;
			using DenType = typename RatioTraits<PromotedQ>::DenType;
			return PromotedQ{
				(NumType(lhs.num) * rhs.den) + (NumType(rhs.num) * lhs.den),
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
			requires Rational<Lhs> && Rational<Rhs>
		constexpr inline auto subRatio(const Lhs lhs, const Rhs rhs){
			constexpr auto LhsN = RatioTraits<Lhs>::numBits;
			constexpr auto RhsN = RatioTraits<Rhs>::numBits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename PromotedT<GreaterQ>::Type;
			using NumType = typename RatioTraits<PromotedQ>::NumType;
			using DenType = typename RatioTraits<PromotedQ>::DenType;
			return PromotedQ{
				(NumType(lhs.num) * rhs.den) - (NumType(rhs.num) * lhs.den),
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
			requires Rational<Lhs> && Rational<Rhs>
		constexpr inline auto mulRatio(const Lhs lhs, const Rhs rhs){
			constexpr auto LhsN = RatioTraits<Lhs>::numBits;
			constexpr auto RhsN = RatioTraits<Rhs>::numBits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename PromotedT<GreaterQ>::Type;
			using NumType = typename RatioTraits<PromotedQ>::NumType;
			using DenType = typename RatioTraits<PromotedQ>::DenType;
			return PromotedQ{
				NumType(lhs.num) * rhs.num,
				DenType(lhs.den) * rhs.den
			};
		}

		template<typename Lhs, typename Rhs>
			requires Rational<Lhs> && Rational<Rhs>
		constexpr inline auto divRatio(const Lhs lhs, const Rhs rhs){
			constexpr auto LhsN = RatioTraits<Lhs>::numBits;
			constexpr auto RhsN = RatioTraits<Rhs>::numBits;
			using GreaterQ = std::conditional_t<(LhsN > RhsN), Lhs, Rhs>;
			using PromotedQ = typename PromotedT<GreaterQ>::Type;
			using NumType = typename RatioTraits<PromotedQ>::NumType;
			using DenType = typename RatioTraits<PromotedQ>::DenType;

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
		return reinterpret_cast<detail::FnFunctorType<FnType>>(f.fptr)(f.env, std::forward<Args>(args)...);
	}

	template<typename T>
	using Promoted = typename detail::PromotedT<T>::Type;

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
	using Ratio = typename detail::RatioT<N>::Type;

	using Ratio32 = Ratio<32>;
	using Ratio64 = Ratio<64>;
	using Ratio128 = Ratio<128>;

	template<typename T>
	constexpr auto IsRatio = detail::IsRatioT<T>::value;

	template<typename Q>
	using RatioNumType = typename detail::RatioTraits<Q>::NumType;

	template<typename Q>
	using RatioDenType = typename detail::RatioTraits<Q>::DenType;

	template<size_t N>
	using Real = typename detail::RealT<N>::Type;

	using Real32 = Real<32>;
	using Real64 = Real<64>;

	using Str = LemniStr;

	template<typename...>
	class Functor;

	template<typename Ret, typename ... Params>
	class Functor<Ret(Params...)>{
		public:
			explicit Functor(Ret(*fptr)(Params...)) noexcept{
				functor.dtor = [](auto...){};
				functor.fptr = fptr;
				functor.env = nullptr;
			}

			template<typename Env>
			Functor(Env env, Ret(*fptr)(const Env*, Params...)){
				functor.dtor = [](void *ptr){
					auto envptr = reinterpret_cast<Env*>(ptr);
					delete envptr;
				};

				functor.fptr = reinterpret_cast<void(*)()>(fptr);

				functor.env = new Env(std::move(env));
			}

			Functor(Functor &&other) noexcept{
				functor.dtor = other.functor.dtor;
				functor.fptr = other.functor.fptr;
				functor.env = other.functor.env;

				other.functor.dtor = [](auto...){};
				other.functor.fptr = [](auto...){};
				other.functor.env = nullptr;
			}

			~Functor(){
				functor.dtor(functor.env);
			}

			Functor &operator=(Functor &&other) noexcept{
				functor.dtor(functor.env);

				functor.dtor = other.functor.dtor;
				functor.fptr = other.functor.fptr;
				functor.env = other.functor.env;

				other.functor.dtor = [](auto...){};
				other.functor.fptr = [](auto...){};
				other.functor.env = nullptr;

				return *this;
			}

			template<typename ... Args>
			Ret call(Args &&... args) const{
				return reinterpret_cast<Ret(*)(const void*, Params...)>(functor.fptr)(functor.env, std::forward<Args>(args)...);
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

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline auto operator +(const Lhs lhs, const Rhs rhs) noexcept{
	return lemni::interop::detail::addRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline auto operator -(const Lhs lhs, const Rhs rhs) noexcept{
	return lemni::interop::detail::subRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline auto operator *(const Lhs lhs, const Rhs rhs) noexcept{
	return lemni::interop::detail::mulRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline auto operator /(const Lhs lhs, const Rhs rhs) noexcept{
	return lemni::interop::detail::divRatio(lhs, rhs);
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator <(const Lhs lhs, const Rhs rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot < rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem < rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator >(const Lhs lhs, const Rhs rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot > rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem > rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator <=(const Lhs lhs, const Rhs rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot < rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem <= rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator >=(const Lhs lhs, const Rhs rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot > rhsDiv.quot) || ((lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem >= rhsDiv.rem));
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator ==(const Lhs lhs, const Rhs rhs) noexcept{
	auto lhsDiv = std::div(lhs.num, lhs.den);
	auto rhsDiv = std::div(rhs.num, rhs.den);
	return (lhsDiv.quot == rhsDiv.quot) && (lhsDiv.rem == rhsDiv.rem);
}

template<typename Lhs, typename Rhs>
	requires lemni::interop::detail::Rational<Lhs> && lemni::interop::detail::Rational<Rhs>
inline bool operator !=(const Lhs lhs, const Rhs rhs) noexcept{
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
