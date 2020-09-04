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

#ifndef LEMNI_AREAL_H
#define LEMNI_AREAL_H 1

#include "ARatio.h"

/**
 * @defgroup AReal Arbitraty-precision real numbers
 * Any functions returning a new \ref LemniAReal must be followed by calls to \ref lemniDestroyAReal .
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniARealT *LemniAReal;
typedef const struct LemniARealT *LemniARealConst;

LemniAReal lemniCreateAReal(void);
LemniAReal lemniCreateARealCopy(LemniARealConst other);

/**
 * @brief Create an arbitrary-precision real from a string.
 * @param str string containing real literal
 * @param base base of the numeric string
 * @returns handle of newly created AReal
 */
LemniAReal lemniCreateARealStr(const LemniStr str, const LemniNat16 base);

LemniAReal lemniCreateARealAInt(LemniAIntConst aint);
LemniAReal lemniCreateARealARatio(LemniARatioConst aratio);
LemniAReal lemniCreateARealDouble(const LemniReal64 d);
LemniAReal lemniCreateARealLong(const LemniInt64 si);
LemniAReal lemniCreateARealULong(const LemniNat64 ui);

/**
 * @brief Destroy an arbitrary-precision real.
 * @param areal handle of AReal to destroy
 */
void lemniDestroyAReal(LemniAReal areal);

/**
 * @brief Perform the operation ``\p res = \p other`` .
 * @param res handle to store result in
 * @param other handle to value
 */
void lemniARealSet(LemniAReal res, LemniARealConst other);

typedef void(*LemniARealStrCB)(void *user, const LemniStr str);

void lemniARealStr(LemniARealConst areal, void *user, LemniARealStrCB cb);

bool lemniARealRoundsToFloat(LemniARealConst areal);
bool lemniARealRoundsToDouble(LemniARealConst areal);

float lemniARealToFloat(LemniARealConst areal);
double lemniARealToDouble(LemniARealConst areal);

uint32_t lemniARealNumIntBits(LemniARealConst areal);
uint32_t lemniARealNumFracBits(LemniARealConst areal);

/**
 * @brief Perform addition: ``\p res = \p lhs + \p rhs`` .
 * @param res handle to store result in
 * @param lhs left-hand side of operation
 * @param rhs right-hand side of operation
 */
void lemniARealAdd(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

/**
 * @brief Perform subtraction: ``\p res = \p lhs - \p rhs`` .
 * @param res handle to store result in
 * @param lhs left-hand side of operation
 * @param rhs right-hand side of operation
 */
void lemniARealSub(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

/**
 * @brief Perform multiplication:``\p res = \p lhs * \p rhs``
 * @param res handle to store result in
 * @param lhs left-hand side of operation
 * @param rhs right-hand side of operation
 */
void lemniARealMul(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

/**
 * @brief Perform division: ``\p res = \p lhs / \p rhs`` .
 * @param res handle to store result in
 * @param lhs left-hand side of operation
 * @param rhs right-hand side of operation
 */
void lemniARealDiv(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

/**
 * @brief Get the power function: ``\p res = pow \p lhs \p rhs`` .
 * @param res handle to store result in
 * @param lhs left-hand side of operation
 * @param rhs right-hand side of operation
 */
void lemniARealPow(LemniAReal res, LemniARealConst base, LemniARealConst exp);

/**
 * @brief Get the negative of a value: ``\p res = -\p val``
 * @param res handle to store result in
 * @param val handle to value
 */
void lemniARealNeg(LemniAReal res, LemniARealConst val);

/**
 * @brief Get the absolute value of a number: ``\p res = abs \p val``
 * @param res
 * @param val
 */
void lemniARealAbs(LemniAReal res, LemniARealConst val);

int lemniARealCmp(LemniARealConst lhs, LemniARealConst rhs);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <string_view>

namespace lemni{
	class AReal{
		public:
			AReal() noexcept: m_val(lemniCreateAReal()){}

			explicit AReal(std::string_view str, const LemniNat16 base = 10) noexcept
				: m_val(lemniCreateARealStr(LemniStr{ str.data(), str.size() }, base)){}

			explicit AReal(const AInt &aint) noexcept
				: m_val(lemniCreateARealAInt(aint.m_val)){}

			explicit AReal(const ARatio &aratio) noexcept
				: m_val(lemniCreateARealARatio(aratio.m_val)){}

			explicit AReal(const LemniReal64 d) noexcept
				: m_val(lemniCreateARealDouble(d)){}

			explicit AReal(const LemniReal32 f) noexcept
				: AReal(static_cast<LemniReal64>(f)){}

			explicit AReal(const LemniInt64 si) noexcept
				: m_val(lemniCreateARealLong(si)){}

			explicit AReal(const LemniNat64 ui) noexcept
				: m_val(lemniCreateARealULong(ui)){}

			explicit AReal(const LemniInt32 si) noexcept
				: AReal(static_cast<LemniInt64>(si)){}

			explicit AReal(const LemniNat32 ui) noexcept
				: AReal(static_cast<LemniNat64>(ui)){}

			AReal(const AReal &other) noexcept
				: m_val(lemniCreateARealCopy(other.m_val)){}

			AReal(AReal &&other) noexcept
				: m_val(other.m_val)
			{
				other.m_val = nullptr;
			}

			~AReal(){ if(m_val) lemniDestroyAReal(m_val); }

			AReal &operator =(const AReal &other) noexcept{
				lemniARealSet(m_val, other.m_val);
				return *this;
			}

			AReal &operator =(AReal &&other) noexcept{
				if(m_val) lemniDestroyAReal(m_val);
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			//operator LemniAReal() noexcept{ return m_val; }
			operator LemniARealConst() const noexcept{ return m_val; }

			static AReal from(LemniAReal areal) noexcept{
				return AReal(areal);
			}

			//LemniAReal handle() noexcept{ return m_val; }
			LemniARealConst handle() const noexcept{ return m_val; }

			std::string toString() const noexcept{
				std::string res;
				lemniARealStr(m_val, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

			bool roundsToFloat() const noexcept{ return lemniARealRoundsToFloat(m_val); }

			bool roundsToDouble() const noexcept{ return lemniARealRoundsToDouble(m_val); }

			float toFloat() const noexcept{ return lemniARealToFloat(m_val); }

			double toDouble() const noexcept{ return lemniARealToDouble(m_val); }

			std::uint32_t numIntBits() const noexcept{ return lemniARealNumIntBits(m_val); }

			std::uint32_t numFracBits() const noexcept{ return lemniARealNumFracBits(m_val); }

			AReal operator -() const noexcept{
				AReal res;
				lemniARealNeg(res.m_val, m_val);
				return res;
			}

			AReal &operator +=(const AReal &rhs) noexcept{
				lemniARealAdd(m_val, m_val, rhs.m_val);
				return *this;
			}

			AReal &operator -=(const AReal &rhs) noexcept{
				lemniARealSub(m_val, m_val, rhs.m_val);
				return *this;
			}

			AReal &operator *=(const AReal &rhs) noexcept{
				lemniARealMul(m_val, m_val, rhs.m_val);
				return *this;
			}

			AReal &operator /=(const AReal &rhs) noexcept{
				lemniARealDiv(m_val, m_val, rhs.m_val);
				return *this;
			}

			AReal operator +(const AReal &rhs) const noexcept{
				AReal res;
				lemniARealAdd(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AReal operator -(const AReal &rhs) const noexcept{
				AReal res;
				lemniARealSub(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AReal operator *(const AReal &rhs) const noexcept{
				AReal res;
				lemniARealMul(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AReal operator /(const AReal &rhs) const noexcept{
				AReal res;
				lemniARealDiv(res.m_val, m_val, rhs.m_val);
				return res;
			}

			bool operator <(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) < 0;
			}

			bool operator >(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) > 0;
			}

			bool operator <=(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) <= 0;
			}

			bool operator >=(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) >= 0;
			}

			bool operator ==(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) == 0;
			}

			bool operator !=(const AReal &rhs) const noexcept{
				return lemniARealCmp(m_val, rhs.m_val) != 0;
			}

		private:
			AReal(LemniAReal areal) noexcept
				: m_val(areal){}

			LemniAReal m_val;

			friend AReal abs(AReal val) noexcept;
	};

	inline AReal abs(AReal val) noexcept{
		lemniARealAbs(val.m_val, val.m_val);
		return val;
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_AREAL_H
