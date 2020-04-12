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

#ifndef LEMNI_ARATIO_H
#define LEMNI_ARATIO_H 1

#include "AInt.h"
#include "Interop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniARatioT *LemniARatio;
typedef const struct LemniARatioT *LemniARatioConst;

LemniARatio lemniCreateARatio(void);
LemniARatio lemniCreateARatioCopy(LemniARatioConst other);
LemniARatio lemniCreateARatioStr(const LemniStr str, const int base);
LemniARatio lemniCreateARatioAInt(LemniAIntConst num, LemniAIntConst den);
LemniARatio lemniCreateARatioLong(const long num, const unsigned long den);
LemniARatio lemniCreateARatioULong(const unsigned long num, const unsigned long den);
LemniARatio lemniCreateARatioFrom32(const LemniRatio32 q32);
LemniARatio lemniCreateARatioFrom64(const LemniRatio64 q64);
LemniARatio lemniCreateARatioFrom128(const LemniRatio128 q128);

void lemniDestroyARatio(LemniARatio aratio);

void lemniARatioSet(LemniARatio res, LemniARatioConst other);

typedef struct {
	uint64_t num;
	uint64_t den;
} LemniARatioNumBitsResult;

LemniARatioNumBitsResult lemniARatioNumBits(LemniARatioConst aratio);

typedef void(*LemniARatioStrCB)(void *user, const LemniStr str);

void lemniARatioStr(LemniARatioConst aratio, void *user, LemniARatioStrCB cb);

LemniAInt lemniARatioNum(LemniARatioConst aratio);
LemniAInt lemniARatioDen(LemniARatioConst aratio);

void lemniARatioAdd(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);
void lemniARatioSub(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);
void lemniARatioMul(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);
void lemniARatioDiv(LemniARatio res, LemniARatioConst lhs, LemniARatioConst rhs);

void lemniARatioInv(LemniARatio res, LemniARatioConst val);
void lemniARatioNeg(LemniARatio res, LemniARatioConst val);
void lemniARatioAbs(LemniARatio res, LemniARatioConst val);

int lemniARatioCmp(LemniARatioConst lhs, LemniARatioConst rhs);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <string_view>

namespace lemni{
	class ARatio{
		public:
			ARatio(): m_val(lemniCreateARatio()){}

			explicit ARatio(std::string_view str, const int base = 10) noexcept
				: m_val(lemniCreateARatioStr(LemniStr{ str.data(), str.size() }, base)){}

			ARatio(const AInt &num, const AInt &den) noexcept
				: m_val(lemniCreateARatioAInt(num.m_val, den.m_val)){}

			ARatio(const long num, const unsigned long den) noexcept
				: m_val(lemniCreateARatioLong(num, den)){}

			ARatio(const unsigned long num, const unsigned long den) noexcept
				: m_val(lemniCreateARatioULong(num, den)){}

			explicit ARatio(const LemniRatio32 &q32) noexcept
				: m_val(lemniCreateARatioFrom32(q32)){}

			explicit ARatio(const LemniRatio64 &q64) noexcept
				: m_val(lemniCreateARatioFrom64(q64)){}

			explicit ARatio(const LemniRatio128 &q128) noexcept
				: m_val(lemniCreateARatioFrom128(q128)){}

			ARatio(const ARatio &other) noexcept
				: m_val(lemniCreateARatioCopy(other.m_val)){}

			ARatio(ARatio &&other) noexcept
				: m_val(other.m_val)
			{
				other.m_val = nullptr;
			}

			~ARatio(){ if(m_val) lemniDestroyARatio(m_val); }

			ARatio &operator =(const ARatio &other) noexcept{
				lemniARatioSet(m_val, other.m_val);
				return *this;
			}

			ARatio &operator =(ARatio &&other) noexcept{
				m_val = other.m_val;
				other.m_val = nullptr;
				return *this;
			}

			static ARatio from(LemniARatio aratio) noexcept{
				return ARatio(aratio);
			}

			LemniARatio handle() noexcept{ return m_val; }
			LemniARatioConst handle() const noexcept{ return m_val; }

			lemni::AInt num() const noexcept{
				return lemni::AInt::from(lemniARatioNum(m_val));
			}

			lemni::AInt den() const noexcept{
				return lemni::AInt::from(lemniARatioDen(m_val));
			}

			LemniARatioNumBitsResult numBits() const noexcept{
				return lemniARatioNumBits(m_val);
			}

			std::string toString() const noexcept{
				std::string res;
				lemniARatioStr(m_val, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

			ARatio operator -() const noexcept{
				ARatio res;
				lemniARatioNeg(res.m_val, m_val);
				return res;
			}

			ARatio &operator +=(const ARatio &rhs) noexcept{
				lemniARatioAdd(m_val, m_val, rhs.m_val);
				return *this;
			}

			ARatio &operator -=(const ARatio &rhs) noexcept{
				lemniARatioSub(m_val, m_val, rhs.m_val);
				return *this;
			}

			ARatio &operator *=(const ARatio &rhs) noexcept{
				lemniARatioMul(m_val, m_val, rhs.m_val);
				return *this;
			}

			ARatio &operator /=(const ARatio &rhs) noexcept{
				lemniARatioDiv(m_val, m_val, rhs.m_val);
				return *this;
			}

			ARatio operator +(const ARatio &rhs) const noexcept{
				ARatio res;
				lemniARatioAdd(res.m_val, m_val, rhs.m_val);
				return res;
			}

			ARatio operator -(const ARatio &rhs) const noexcept{
				ARatio res;
				lemniARatioSub(res.m_val, m_val, rhs.m_val);
				return res;
			}

			ARatio operator *(const ARatio &rhs) const noexcept{
				ARatio res;
				lemniARatioMul(res.m_val, m_val, rhs.m_val);
				return res;
			}

			ARatio operator /(const ARatio &rhs) const noexcept{
				ARatio res;
				lemniARatioDiv(res.m_val, m_val, rhs.m_val);
				return res;
			}

			bool operator <(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) < 0;
			}

			bool operator >(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) > 0;
			}

			bool operator <=(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) <= 0;
			}

			bool operator >=(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) >= 0;
			}

			bool operator ==(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) == 0;
			}

			bool operator !=(const ARatio &rhs) const noexcept{
				return lemniARatioCmp(m_val, rhs.m_val) != 0;
			}

		private:
			explicit ARatio(LemniARatio aratio) noexcept
				: m_val(aratio){}

			LemniARatio m_val;

			friend ARatio abs(ARatio val) noexcept;
			friend ARatio inv(ARatio val) noexcept;
			friend class AReal;
	};

	inline ARatio abs(ARatio val) noexcept{
		lemniARatioAbs(val.m_val, val.m_val);
		return val;
	}

	inline ARatio inv(ARatio val) noexcept{
		lemniARatioInv(val.m_val, val.m_val);
		return val;
	}
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_ARATIO_H
