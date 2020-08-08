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
#include "TypedExpr.h"
#include "Module.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef const struct LemniValueT *LemniValue;
typedef const struct LemniValueT *LemniValueConst;

typedef struct LemniValueCallErrorT{
	LemniStr msg;
} LemniValueCallError;

typedef struct LemniValueCallResultT{
	bool hasError;
	union {
		LemniValueCallError error;
		LemniValue value;
	};
} LemniValueCallResult;

typedef struct LemniEvalStateT *LemniEvalState;
typedef struct LemniEvalBindingsT *LemniEvalBindings;

typedef struct LemniValueBindingsT *LemniValueBindings;
typedef const struct LemniValueBindingsT *LemniValueBindingsConst;

LemniValueBindings lemniCreateValueBindings();

void lemniDestroyValueBindings(LemniValueBindings bindings);

void lemniSetValueBinding(LemniValueBindings bindings, const LemniStr name, LemniValue value);

LemniValue lemniGetValueBinding(LemniValueBindings bindings, const LemniStr name);

typedef LemniType(*LemniTypeFn)(void *const ptr, LemniTypeSet types);
typedef LemniValueCallResult(*LemniEvalFn)(void *const ptr, LemniEvalState state, LemniEvalBindings bindings, LemniValue *const args, const LemniNat64 numArgs);

void lemniDestroyValue(LemniValue value);

LemniValue lemniCreateValueCopy(LemniValue value);

LemniValue lemniCreateValueRef(LemniValue value);

LemniValue lemniCreateValueModule(LemniModule handle);

LemniValue lemniCreateValueUnit(void);

LemniValue lemniCreateValueBool(const bool b);

LemniValue lemniCreateValueFn(LemniTypeFn typeFn, LemniEvalFn fn, void *const ptr, LemniEvalState state, LemniEvalBindings bindings);

LemniValue lemniCreateValueProduct(const LemniValueConst *const vals, const LemniNat64 numVals);

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

LemniValue lemniCreateValueStrUTF8(const LemniStr str);
LemniValue lemniCreateValueStrASCII(const LemniStr str);

LemniValue lemniCreateValueType(LemniType type);

typedef void(*LemniValueStrCB)(void *user, LemniStr str);

void lemniValueStr(LemniValue value, void *user, LemniValueStrCB cb);

LemniValueCallResult lemniValueCall(LemniValue fn, LemniValue *const args, const LemniNat32 numArgs);

LemniValue lemniValueAccess(LemniValue val, LemniStr member);

LemniInt16 lemniValueIsTrue(LemniValueConst val);
LemniInt16 lemniValueIsFalse(LemniValueConst val);

LemniValue lemniValueUnaryOp(const LemniUnaryOp op, LemniValue value);
LemniValue lemniValueBinaryOp(const LemniBinaryOp op, LemniValue lhs, LemniValue rhs);

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
#include <vector>
#include <utility>

namespace lemni{
	namespace detail{
		template<typename...>
		struct ValueCreator;

		template<> struct ValueCreator<LemniUnit>{ static LemniValue create(LemniUnit){ return lemniCreateValueUnit(); } };

		template<> struct ValueCreator<bool>{ static LemniValue create(bool v){ return lemniCreateValueBool(v); } };
		template<> struct ValueCreator<LemniBool>{ static LemniValue create(LemniBool v){ return lemniCreateValueBool(v == LEMNI_TRUE); } };

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

		template<> struct ValueCreator<LemniStr>{ static LemniValue create(const LemniStr v){ return lemniCreateValueStrUTF8(v); } };

		template<typename T>
		LemniValue createLemniValue(T v){ return ValueCreator<T>::create(v); }
	}

	class Value{
		public:
			Value() noexcept: m_value(nullptr){}

			template<typename T>
			explicit Value(T value) noexcept
				: m_value(detail::ValueCreator<T>::create(value)){}

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

			LemniValue release() noexcept{
				auto ret = m_value;
				m_value = nullptr;
				return ret;
			}

			std::string toString() const noexcept{
				std::string res;
				lemniValueStr(m_value, &res, [](void *p, const LemniStr str){
					auto strp = reinterpret_cast<std::string*>(p);
					*strp = std::string(str.ptr, str.len);
				});
				return res;
			}

			Value doCall(LemniValue *const args, const LemniNat32 numArgs){
				auto callRes = lemniValueCall(m_value, args, numArgs);
				if(callRes.hasError){
					throw std::runtime_error(lemni::toStdStr(callRes.error.msg));
				}
				else{
					return Value::from(callRes.value);
				}
			}

			template<typename Arg, typename ... Args>
			Value call(Arg &&arg, Args &&... args){
				LemniValue argVals[] = {
					detail::createLemniValue(std::forward<Arg>(arg)),
					detail::createLemniValue(std::forward<Args>(args))...
				};

				return doCall(argVals, sizeof...(Args) + 1);
			}

			template<typename Arg, typename ... Args>
			Value operator()(Arg &&arg, Args &&... args){
				return call(std::forward<Arg>(arg), std::forward<Args>(args)...);
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

	class ValueBindings{
		public:
			ValueBindings() noexcept
				: m_bindings(lemniCreateValueBindings()){}

			ValueBindings(ValueBindings &&other) noexcept
				: m_bindings(other.m_bindings)
			{
				other.m_bindings = nullptr;
			}

			~ValueBindings(){ if(m_bindings) lemniDestroyValueBindings(m_bindings); }

			ValueBindings &operator =(ValueBindings &&other) noexcept{
				m_bindings = other.m_bindings;
				other.m_bindings = nullptr;
				return *this;
			}

			LemniValueBindings handle() noexcept{ return m_bindings; }

			void set(LemniStr name, LemniValue val){
				lemniSetValueBinding(m_bindings, name, val);
			}

			void set(std::string_view name, Value val){
				set(lemni::fromStdStrView(name), val.release());
			}

			LemniValue get(LemniStr name) const{
				return lemniGetValueBinding(m_bindings, name);
			}

			Value get(std::string_view name) const{
				auto res = get(lemni::fromStdStrView(name));
				return Value::from(res);
			}

		private:
			LemniValueBindings m_bindings;
	};
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_VALUE_H
