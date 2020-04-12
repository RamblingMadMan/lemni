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

#ifndef LEMNI_VALUE_H
#define LEMNI_VALUE_H 1

#include "Interop.h"
#include "AReal.h"
#include "Operator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct LemniValueT *LemniValue;

void lemniDestroyValue(LemniValue value);

LemniValue lemniCreateValueCopy(LemniValue value);

LemniValue lemniCreateValueRef(LemniValue value);

LemniValue lemniCreateValueUnit(void);

LemniValue lemniCreateValueBool(const LemniBool b);

LemniValue lemniCreateValueNat16(const LemniNat16 n16);
LemniValue lemniCreateValueNat32(const LemniNat32 n32);
LemniValue lemniCreateValueNat64(const LemniNat64 n64);
LemniValue lemniCreateValueANat(LemniAIntConst aint);

LemniValue lemniCreateValueInt16(const LemniInt16 i16);
LemniValue lemniCreateValueInt32(const LemniInt32 i32);
LemniValue lemniCreateValueInt64(const LemniInt64 i64);
LemniValue lemniCreateValueAInt(LemniAIntConst aint);

LemniValue lemniCreateValueRatio32(const LemniRatio32 q32);
LemniValue lemniCreateValueRatio64(const LemniRatio64 q64);
LemniValue lemniCreateValueRatio128(const LemniRatio128 q128);
LemniValue lemniCreateValueARatio(LemniARatioConst aratio);

LemniValue lemniCreateValueReal32(const LemniReal32 r32);
LemniValue lemniCreateValueReal64(const LemniReal64 r64);
LemniValue lemniCreateValueAReal(LemniARealConst areal);

typedef void(*LemniValueStrCB)(void *user, LemniStr str);

void lemniValueStr(LemniValue value, void *user, LemniValueStrCB cb);

LemniValue lemniValueBinaryOp(LemniBinaryOp op, LemniValue lhs, LemniValue rhs);

LemniValue lemniValueAdd(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueSub(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueMul(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueDiv(LemniValue lhs, LemniValue rhs);

LemniValue lemniValueAnd(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueOr(LemniValue lhs, LemniValue rhs);

LemniValue lemniValueLessThan(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueGreaterThan(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueLessThanEqual(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueGreaterThanEqual(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueEqual(LemniValue lhs, LemniValue rhs);
LemniValue lemniValueNotEqual(LemniValue lhs, LemniValue rhs);

#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <stdexcept>
#include <utility>

namespace lemni{
	namespace detail{
		template<typename...>
		struct ValueCreator;

		template<> struct ValueCreator<LemniUnit>{ static LemniValue create(LemniUnit){ return lemniCreateValueUnit(); } };

		template<> struct ValueCreator<LemniBool>{ static LemniValue create(LemniBool v){ return lemniCreateValueBool(v); } };

		template<> struct ValueCreator<LemniNat16>{ static LemniValue create(LemniNat16 v){ return lemniCreateValueNat16(v); } };
		template<> struct ValueCreator<LemniNat32>{ static LemniValue create(LemniNat32 v){ return lemniCreateValueNat32(v); } };
		template<> struct ValueCreator<LemniNat64>{ static LemniValue create(LemniNat64 v){ return lemniCreateValueNat64(v); } };

		template<> struct ValueCreator<LemniInt16>{ static LemniValue create(LemniInt16 v){ return lemniCreateValueInt16(v); } };
		template<> struct ValueCreator<LemniInt32>{ static LemniValue create(LemniInt32 v){ return lemniCreateValueInt32(v); } };
		template<> struct ValueCreator<LemniInt64>{ static LemniValue create(LemniInt64 v){ return lemniCreateValueInt64(v); } };
		template<> struct ValueCreator<lemni::AInt>{ static LemniValue create(const lemni::AInt &v){ return lemniCreateValueAInt(v.handle()); } };

		template<> struct ValueCreator<LemniRatio32>{ static LemniValue create(LemniRatio32 v){ return lemniCreateValueRatio32(v); } };
		template<> struct ValueCreator<LemniRatio64>{ static LemniValue create(LemniRatio64 v){ return lemniCreateValueRatio64(v); } };
		template<> struct ValueCreator<LemniRatio128>{ static LemniValue create(LemniRatio128 v){ return lemniCreateValueRatio128(v); } };
		template<> struct ValueCreator<lemni::ARatio>{ static LemniValue create(const lemni::ARatio &v){ return lemniCreateValueARatio(v.handle()); } };

		template<> struct ValueCreator<LemniReal32>{ static LemniValue create(LemniReal32 v){ return lemniCreateValueReal32(v); } };
		template<> struct ValueCreator<LemniReal64>{ static LemniValue create(LemniReal64 v){ return lemniCreateValueReal64(v); } };
		template<> struct ValueCreator<lemni::AReal>{ static LemniValue create(const lemni::AReal &v){ return lemniCreateValueAReal(v.handle()); } };

		template<typename T>
		LemniValue createLemniValue(T v){ return ValueCreator<T>::create(v); }
	}

	class Value{
		public:
			template<typename T>
			explicit Value(T value) noexcept
				: m_value(detail::ValueCreator<T>::create(value)){}

			explicit Value(lemni::interop::Unit) noexcept
				: Value(LemniUnit{}){}

			Value(Value &&other) noexcept
				: m_value(other.m_value)
			{
				other.m_value = nullptr;
			}

			Value(const Value &other) noexcept
				: m_value(lemniCreateValueCopy(other.m_value)){}

			~Value(){ if(m_value) lemniDestroyValue(m_value); }

			Value &operator=(Value &&other) noexcept{
				if(m_value) lemniDestroyValue(m_value);
				m_value = other.m_value;
				other.m_value = nullptr;
				return *this;
			}

			Value &operator=(const Value &other) noexcept{
				if(m_value) lemniDestroyValue(m_value);
				m_value = lemniCreateValueCopy(other.m_value);
				return *this;
			}

			static Value from(LemniValue value){
				return Value(value);
			}

			LemniValue handle() const noexcept{ return m_value; }

			std::string toString() const noexcept{
				std::string res;
				lemniValueStr(m_value, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

			Value &operator+=(LemniValue rhs) noexcept{
				auto old = std::exchange(m_value, lemniValueAdd(m_value, rhs));
				lemniDestroyValue(old);
				return *this;
			}

			Value &operator-=(LemniValue rhs) noexcept{
				auto old = std::exchange(m_value, lemniValueSub(m_value, rhs));
				lemniDestroyValue(old);
				return *this;
			}

			Value &operator*=(LemniValue rhs) noexcept{
				auto old = std::exchange(m_value, lemniValueMul(m_value, rhs));
				lemniDestroyValue(old);
				return *this;
			}

			Value &operator/=(LemniValue rhs) noexcept{
				auto old = std::exchange(m_value, lemniValueDiv(m_value, rhs));
				lemniDestroyValue(old);
				return *this;
			}

			Value operator+(const Value &rhs) const noexcept{
				return Value(lemniValueAdd(m_value, rhs.m_value));
			}

			Value operator-(const Value &rhs) const noexcept{
				return Value(lemniValueSub(m_value, rhs.m_value));
			}

			Value operator*(const Value &rhs) const noexcept{
				return Value(lemniValueMul(m_value, rhs.m_value));
			}

			Value operator/(const Value &rhs) const noexcept{
				return Value(lemniValueDiv(m_value, rhs.m_value));
			}

		private:
			Value(LemniValue value)
			{
				if(!value){
					throw std::runtime_error("no result defined for operation");
				}
				else{
					m_value = value;
				}
			}

			LemniValue m_value;
	};
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_VALUE_H
