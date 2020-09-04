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

#ifndef LEMNI_AINT_H
#define LEMNI_AINT_H 1

#include "Str.h"
#include "Interop.h"

/**
 * @defgroup AInt Arbitraty-precision integral numbers
 * Any functions returning a new \ref LemniAInt must be followed by calls to \ref lemniDestroyAInt .
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniAIntT *LemniAInt;
typedef const struct LemniAIntT *LemniAIntConst;

LemniAInt lemniCreateAInt(void);
LemniAInt lemniCreateAIntCopy(LemniAIntConst other);
LemniAInt lemniCreateAIntStr(const LemniStr str, const LemniNat16 base);
LemniAInt lemniCreateAIntLong(const LemniInt64 si);
LemniAInt lemniCreateAIntULong(const LemniNat64 ui);

void lemniDestroyAInt(LemniAInt aint);

void lemniAIntSet(LemniAInt aint, LemniAIntConst other);
void lemniAIntSetStr(LemniAInt aint, LemniStr str, const LemniNat16 base);
void lemniAIntSetLong(LemniAInt aint, const LemniInt64 si);
void lemniAIntSetULong(LemniAInt aint, const LemniNat64 ui);

long lemniAIntToLong(LemniAIntConst aint);
unsigned long lemniAIntToULong(LemniAIntConst aint);

uint64_t lemniAIntNumBits(LemniAIntConst aint);
uint64_t lemniAIntNumBitsUnsigned(LemniAIntConst aint);

typedef void(*LemniAIntStrCB)(void *user, const LemniStr str);

void lemniAIntStr(LemniAIntConst aint, void *user, LemniAIntStrCB cb);

void lemniAIntAdd(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntSub(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntMul(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);

void lemniAIntPow(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);

void lemniAIntNeg(LemniAInt res, LemniAIntConst val);
void lemniAIntAbs(LemniAInt res, LemniAIntConst val);

int lemniAIntCmp(LemniAIntConst lhs, LemniAIntConst rhs);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <string_view>

namespace lemni{
	class AInt{
		public:
			AInt() noexcept
				: m_val(lemniCreateAInt()){}

			explicit AInt(std::string_view str, const int base = 10) noexcept
				: m_val(lemniCreateAIntStr(LemniStr{str.data(), str.size()}, base)){}

			explicit AInt(const LemniInt64 si) noexcept
				: m_val(lemniCreateAIntLong(si)){}

			explicit AInt(const LemniNat64 ui) noexcept
				: m_val(lemniCreateAIntULong(ui)){}

			explicit AInt(const LemniInt32 i) noexcept
				: AInt(static_cast<long>(i)){}

			explicit AInt(const LemniNat32 i) noexcept
				: AInt(static_cast<unsigned long>(i)){}

			AInt(const AInt &other) noexcept
				: m_val(lemniCreateAIntCopy(other.m_val)){}

			AInt(AInt &&other) noexcept
				: m_val(other.m_val)
			{
				other.m_val = nullptr;
			}

			~AInt(){ if(m_val) lemniDestroyAInt(m_val); }

			AInt &operator =(const AInt &other){
				lemniAIntSet(m_val, other.m_val);
				return *this;
			}

			AInt &operator =(AInt &&other){
				if(m_val) lemniDestroyAInt(m_val);
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			operator LemniAInt() noexcept{ return m_val; }
			operator LemniAIntConst() const noexcept{ return m_val; }

			static AInt from(LemniAInt aint) noexcept{
				return AInt(aint);
			}

			LemniAInt handle() noexcept{ return m_val; }
			LemniAIntConst handle() const noexcept{ return m_val; }

			uint64_t numBits() const noexcept{ return lemniAIntNumBits(m_val); }
			uint64_t numBitsUnsigned() const noexcept{ return lemniAIntNumBitsUnsigned(m_val); }

			long toLong() const noexcept{
				return lemniAIntToLong(m_val);
			}

			unsigned long toULong() const noexcept{
				return lemniAIntToULong(m_val);
			}

			std::string toString() const noexcept{
				std::string res;
				lemniAIntStr(m_val, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

			AInt operator -() const noexcept{
				AInt res;
				lemniAIntNeg(res.m_val, m_val);
				return res;
			}

			AInt &operator +=(const AInt &rhs) noexcept{
				lemniAIntAdd(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt &operator -=(const AInt &rhs) noexcept{
				lemniAIntSub(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt &operator *=(const AInt &rhs) noexcept{
				lemniAIntMul(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt operator +(const AInt &rhs) const noexcept{
				AInt res;
				lemniAIntAdd(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AInt operator -(const AInt &rhs) const noexcept{
				AInt res;
				lemniAIntSub(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AInt operator *(const AInt &rhs) const noexcept{
				AInt res;
				lemniAIntMul(res.m_val, m_val, rhs.m_val);
				return res;
			}

			bool operator <(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) < 0;
			}

			bool operator >(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) > 0;
			}

			bool operator <=(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) <= 0;
			}

			bool operator >=(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) >= 0;
			}

			bool operator ==(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) == 0;
			}

			bool operator !=(const AInt &rhs) const noexcept{
				return lemniAIntCmp(m_val, rhs.m_val) != 0;
			}

		private:
			explicit AInt(LemniAInt aint) noexcept
				: m_val(aint){}

			LemniAInt m_val;

			friend AInt abs(AInt val) noexcept;
			friend class ARatio;
			friend class AReal;
	};

	inline AInt abs(AInt val) noexcept{
		lemniAIntAbs(val.m_val, val.m_val);
		return val;
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_AINT_H
