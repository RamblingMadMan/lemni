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

typedef struct LemniUnitT{
	char _pad; // guarantee sizeof(LemniUnit) >= 1
} LemniUnit;

using LemniBool = bool;

using LemniNat16 = uint16_t;
using LemniNat32 = uint32_t;
using LemniNat64 = uint64_t;

using LemniInt16 = int16_t;
using LemniInt32 = int32_t;
using LemniInt64 = int64_t;

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
using LemniReal32 = float;
using LemniReal64 = double;
#endif

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

		template<typename Lhs, typename Rhs>
			requires Rational<Lhs> && Rational<Rhs>
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
	}

	template<typename T>
	using Promoted = typename detail::PromotedT<T>::Type;

	struct Unit{
		public:
			Unit() noexcept = default;
			Unit(const Unit&) noexcept = default;

			~Unit() = default;

			Unit &operator=(const Unit&) noexcept = default;

			operator LemniUnit() const noexcept{ return {}; }
	};

	inline constexpr Unit unit;

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
