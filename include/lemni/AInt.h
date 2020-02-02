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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniAIntT *LemniAInt;
typedef const struct LemniAIntT *LemniAIntConst;

LemniAInt lemniCreateAInt(void);
LemniAInt lemniCreateAIntCopy(LemniAIntConst other);
LemniAInt lemniCreateAIntStr(LemniStr str, const int base);
LemniAInt lemniCreateAIntLong(const long si);
LemniAInt lemniCreateAIntULong(const unsigned long ui);

void lemniDestroyAInt(LemniAInt aint);

void lemniAIntSet(LemniAInt aint, LemniAIntConst other);
void lemniAIntSetStr(LemniAInt aint, LemniStr str, const int base);
void lemniAIntSetLong(LemniAInt aint, const long si);
void lemniAIntSetULong(LemniAInt aint, const unsigned long ui);

LemniStr lemniAIntStr(LemniAInt aint);

void lemniAIntAdd(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntSub(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);
void lemniAIntMul(LemniAInt res, LemniAIntConst lhs, LemniAIntConst rhs);

void lemniAIntNeg(LemniAInt res, LemniAIntConst val);
void lemniAIntAbs(LemniAInt res, LemniAIntConst val);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <string_view>

namespace lemni{
	class AInt{
		public:
			AInt()
				: m_val(lemniCreateAInt()){}

			AInt(std::string_view str, const int base = 10)
				: m_val(lemniCreateAIntStr(LemniStr{str.data(), str.size()}, base)){}

			explicit AInt(const long si)
				: m_val(lemniCreateAIntLong(si)){}

			explicit AInt(const unsigned long ui)
				: m_val(lemniCreateAIntULong(ui)){}

			AInt(const AInt &other)
				: m_val(lemniCreateAIntCopy(other.m_val)){}

			AInt(AInt &&other)
				: m_val(other.m_val)
			{
				other.m_val = nullptr;
			}

			~AInt(){ if(m_val) lemniDestroyAInt(m_val); }

			AInt &operator=(const AInt &other){
				lemniAIntSet(m_val, other.m_val);
				return *this;
			}

			AInt &operator=(AInt &&other){
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			AInt &operator+=(const AInt &rhs) noexcept{
				lemniAIntAdd(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt &operator-=(const AInt &rhs) noexcept{
				lemniAIntSub(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt &operator*=(const AInt &rhs) noexcept{
				lemniAIntMul(m_val, m_val, rhs.m_val);
				return *this;
			}

			AInt operator+(const AInt &rhs) noexcept{
				AInt res;
				lemniAIntAdd(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AInt operator-(const AInt &rhs) noexcept{
				AInt res;
				lemniAIntSub(res.m_val, m_val, rhs.m_val);
				return res;
			}

			AInt operator*(const AInt &rhs) noexcept{
				AInt res;
				lemniAIntMul(res.m_val, m_val, rhs.m_val);
				return res;
			}

		private:
			LemniAInt m_val;

			friend AInt neg(AInt val);
			friend AInt abs(AInt val);
	};

	inline AInt neg(AInt val){
		lemniAIntNeg(val.m_val, val.m_val);
		return val;
	}

	inline AInt abs(AInt val){
		lemniAIntAbs(val.m_val, val.m_val);
		return val;
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_AINT_H
