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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniARealT *LemniAReal;
typedef const struct LemniARealT *LemniARealConst;

LemniAReal lemniCreateAReal(void);
LemniAReal lemniCreateARealCopy(LemniARealConst other);
LemniAReal lemniCreateARealStr(const LemniStr str, const int base);
LemniAReal lemniCreateARealAInt(LemniAIntConst aint);
LemniAReal lemniCreateARealARatio(LemniARatioConst aratio);
LemniAReal lemniCreateARealDouble(const double d);
LemniAReal lemniCreateARealLong(const long si);
LemniAReal lemniCreateARealULong(const unsigned long ui);

void lemniDestroyAReal(LemniAReal areal);

void lemniARealSet(LemniAReal res, LemniARealConst other);

typedef void(*LemniARealStrCB)(void *user, const LemniStr str);

void lemniARealStr(LemniARealConst areal, void *user, LemniARealStrCB cb);

void lemniARealAdd(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealSub(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealMul(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);
void lemniARealDiv(LemniAReal res, LemniARealConst lhs, LemniARealConst rhs);

void lemniARealNeg(LemniAReal res, LemniARealConst val);
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

			explicit AReal(std::string_view str, const int base = 10) noexcept
				: m_val(lemniCreateARealStr(LemniStr{ str.data(), str.size() }, base)){}

			explicit AReal(const AInt &aint) noexcept
				: m_val(lemniCreateARealAInt(aint.m_val)){}

			explicit AReal(const ARatio &aratio) noexcept
				: m_val(lemniCreateARealARatio(aratio.m_val)){}

			explicit AReal(const double d) noexcept
				: m_val(lemniCreateARealDouble(d)){}

			explicit AReal(const long si) noexcept
				: m_val(lemniCreateARealLong(si)){}

			explicit AReal(const unsigned long ui) noexcept
				: m_val(lemniCreateARealULong(ui)){}

			explicit AReal(const int si) noexcept
				: AReal(static_cast<long>(si)){}

			explicit AReal(const unsigned int ui) noexcept
				: AReal(static_cast<unsigned long>(ui)){}

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
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			static AReal from(LemniAReal areal) noexcept{
				return AReal(areal);
			}

			LemniAReal handle() noexcept{ return m_val; }
			LemniARealConst handle() const noexcept{ return m_val; }

			std::string toString() const noexcept{
				std::string res;
				lemniARealStr(m_val, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

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
