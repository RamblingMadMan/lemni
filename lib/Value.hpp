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

#ifndef LEMNI_LIB_VALUE_HPP
#define LEMNI_LIB_VALUE_HPP 1

#include <memory>

#include "lemni/Value.h"

#include "TypedExpr.hpp"

namespace {
	using std::to_string;

	std::string to_string(const LemniRatio32 q){
		auto qCanon = lemni::interop::simplifyRatio(q);
		return std::to_string(qCanon.num) + "/" + std::to_string(qCanon.den);
	}

	std::string to_string(const LemniRatio64 q){
		auto qCanon = lemni::interop::simplifyRatio(q);
		return std::to_string(qCanon.num) + "/" + std::to_string(qCanon.den);
	}

	std::string to_string(const LemniRatio128 &q){
		auto qCanon = lemni::interop::simplifyRatio(q);
		return std::to_string(qCanon.num) + "/" + std::to_string(qCanon.den);
	}

	template<typename T, typename ... Args>
	inline auto allocValue(Args &&... args){
		auto mem = std::malloc(sizeof(T));
		return new(mem) T(std::forward<Args>(args)...);
	}
}

struct LemniValueT{
	virtual ~LemniValueT() = default;

	virtual LemniValue copy() const noexcept{ return nullptr; }

	virtual LemniValue deref() const noexcept{ return this; }

	virtual std::size_t numBits() const noexcept = 0;

	virtual std::string toString() const noexcept = 0;

	virtual std::unique_ptr<std::byte[]> toBytes(LemniType type) const noexcept{
		(void)type;
		return nullptr;
	}

	virtual LemniType getType(LemniTypeSet types) const noexcept{
		(void)types;
		return nullptr;
	}

	virtual LemniValueCallResult call(LemniValue *const args, const LemniNat32 numArgs) const noexcept{
		(void)args;
		(void)numArgs;

		LemniValueCallResult res;
		res.hasError = true;
		res.error.msg = LEMNICSTR("value type is not callable");
		return res;
	}

	virtual LemniValue access(const LemniStr memberName) const noexcept{
		(void)memberName;
		return nullptr;
	}

	virtual LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept{
		(void)op;
		return nullptr;
	}

	virtual LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
		(void)op;
		(void)rhs;
		return nullptr;
	}
};

struct LemniValueRefT: LemniValueT{
	LemniValueRefT(LemniValue refed_) noexcept
		: refed(refed_){}

	LemniValue copy() const noexcept override{ return lemniCreateValueRef(refed); }

	LemniValue deref() const noexcept override{ return refed; }

	std::size_t numBits() const noexcept override{ return refed->numBits(); }

	std::string toString() const noexcept override{ return refed->toString(); };

	std::unique_ptr<std::byte[]> toBytes(LemniType type) const noexcept override{ return refed->toBytes(type); }

	LemniType getType(LemniTypeSet types) const noexcept override{ return refed->getType(types); }

	LemniValueCallResult call(LemniValue *const args, const LemniNat32 numArgs) const noexcept override{ return refed->call(args, numArgs); }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override{ return refed->performUnaryOp(op); }

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override{ return refed->performBinaryOp(op, rhs); }

	LemniValue refed;
};

using LemniValueRef = const LemniValueRefT*;

struct LemniValueModuleT: LemniValueT{
	LemniValueModuleT(lemni::Module mod_, lemni::ValueBindings bindings_)
		: mod(std::move(mod_)), bindings(std::move(bindings_)){}

	LemniValue copy() const noexcept override{ return lemniCreateValueUnit(); }

	std::size_t numBits() const noexcept override{ return 0; }

	std::string toString() const noexcept override{ return "MODULE"; }

	LemniValue access(const LemniStr memberName) const noexcept override{
		return bindings.get(memberName);
	}

	lemni::Module mod;
	lemni::ValueBindings bindings;
};

struct LemniValueUnitT: LemniValueT{
	LemniValue copy() const noexcept override{ return lemniCreateValueUnit(); }

	std::size_t numBits() const noexcept override{ return 0; }

	std::string toString() const noexcept override{ return "()"; }

	std::unique_ptr<std::byte[]> toBytes(LemniType type) const noexcept override{
		if(type){
			auto unitType = lemniTypeAsUnit(type);
			if(!unitType) return nullptr;
		}

		auto ret = std::make_unique<std::byte[]>(1);
		ret[0] = std::byte(0);

		return ret;
	}

	LemniType getType(LemniTypeSet types) const noexcept override{ return lemniTypeSetGetUnit(types); }
};

using LemniValueUnit = const LemniValueUnitT*;

struct LemniValueFnT: LemniValueT{
	//LemniValue copy() const noexcept override{ return ; }
	LemniValueFnT(LemniTypeFn typeFn_, LemniEvalFn fn_, void *const ptr_, LemniEvalState state_, LemniEvalBindings bindings_) noexcept
		: typeFn(typeFn_), fn(fn_), ptr(ptr_), state(state_), bindings(bindings_){}

	LemniValue copy() const noexcept override{ return lemniCreateValueFn(typeFn, fn, ptr, state, bindings); }

	std::size_t numBits() const noexcept override{ return 64; }

	std::string toString() const noexcept override{ return "FUNCTION"; };

	LemniType getType(LemniTypeSet types) const noexcept override{ return typeFn(ptr, types); }

	LemniValueCallResult call(LemniValue *const args, const LemniNat32 numArgs) const noexcept override;

	LemniTypeFn typeFn;
	LemniEvalFn fn;
	void *const ptr;
	LemniEvalState state;
	LemniEvalBindings bindings;
};

struct LemniValueBoolT: LemniValueT{
	LemniValueBoolT(LemniBool value_) noexcept: value(value_){}

	LemniValue copy() const noexcept override{ return lemniCreateValueBool(value); }

	std::size_t numBits() const noexcept override{ return sizeof(LemniBool) * CHAR_BIT; }

	std::string toString() const noexcept override{ return value ? "true" : "false"; }

	LemniType getType(LemniTypeSet types) const noexcept override{
		return lemniTypeSetGetBool(types);
	}

	std::unique_ptr<std::byte[]> toBytes(LemniType type) const noexcept override{
		if(type){
			auto boolType = lemniTypeAsBool(type);
			if(!boolType) return nullptr;
		}

		auto ret = std::make_unique<std::byte[]>(sizeof(unsigned long));
		std::memset(ret.get(), value ? ~0 : 0, sizeof(unsigned long));
		//ret[0] = std::byte(value ? 1 : 0);

		return ret;
	}

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	LemniBool value;
};

using LemniValueBool = const LemniValueBoolT*;

struct LemniValueProductT: LemniValueT{
	LemniValueProductT(const LemniValueConst *const values_, const LemniNat64 numValues_) noexcept
	{
		values.reserve(numValues_);
		for(LemniNat64 i = 0; i < numValues_; i++){
			values.emplace_back(values_[i]->copy());
		}
	}

	~LemniValueProductT(){
		for(auto val : values){
			lemniDestroyValue(val);
		}
	}

	LemniValue copy() const noexcept override{
		auto p = values.data();
		return lemniCreateValueProduct(p, values.size());
	}

	std::size_t numBits() const noexcept override{
		std::size_t ret = 0;
		for(auto val : values){
			ret += val->numBits();
		}

		return ret;
	}

	std::string toString() const noexcept override{
		std::string ret = "(" + values[0]->toString();

		for(std::size_t i = 1; i < values.size(); i++){
			ret += ", " + values[i]->toString();
		}

		ret += ")";

		return ret;
	}

	std::vector<LemniValue> values;
};

struct LemniValueNatT: LemniValueT{};
using LemniValueNat = const LemniValueNatT*;

struct LemniValueIntT: LemniValueT{};
using LemniValueInt = const LemniValueIntT*;

struct LemniValueRatioT: LemniValueT{};
using LemniValueRatio = const LemniValueRatioT*;

struct LemniValueRealT: LemniValueT{};
using LemniValueReal = const LemniValueRealT*;

struct LemniValueStrT: LemniValueT{};
using LemniValueStr = const LemniValueStrT*;

struct LemniValueANatT: LemniValueNatT{
	LemniValueANatT(LemniAIntConst value_)
		: value(lemni::AInt::from(lemniCreateAIntCopy(value_))){}

	LemniValue copy() const noexcept override{ return lemniCreateValueANat(value.handle()); }

	std::size_t numBits() const noexcept override{ return SIZE_MAX; }

	std::string toString() const noexcept override{ return value.toString(); }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	lemni::AInt value;
};

using LemniValueANat = const LemniValueANatT*;

struct LemniValueAIntT: LemniValueIntT{
	LemniValueAIntT(LemniAIntConst value_)
		: value(lemni::AInt::from(lemniCreateAIntCopy(value_))){}

	LemniValueAIntT(lemni::AInt value_)
		: value(std::move(value_)){}

	LemniValue copy() const noexcept override{ return lemniCreateValueAInt(value.handle()); }

	std::size_t numBits() const noexcept override{ return SIZE_MAX; }

	std::string toString() const noexcept override{ return value.toString(); }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	lemni::AInt value;
};

using LemniValueAInt = const LemniValueAIntT*;

struct LemniValueARatioT: LemniValueRatioT{
	LemniValueARatioT(LemniARatioConst value_)
		: value(lemni::ARatio::from(lemniCreateARatioCopy(value_))){}

	LemniValueARatioT(lemni::ARatio value_)
		: value(std::move(value_)){}

	LemniValue copy() const noexcept override{ return lemniCreateValueARatio(value.handle()); }

	std::size_t numBits() const noexcept override{ return SIZE_MAX; }

	std::string toString() const noexcept override{ return value.toString(); }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	lemni::ARatio value;
};

using LemniValueARatio = const LemniValueARatioT*;

struct LemniValueARealT: LemniValueRealT{
	LemniValueARealT(LemniARealConst value_)
		: value(lemni::AReal::from(lemniCreateARealCopy(value_))){}

	LemniValueARealT(lemni::AReal value_)
		: value(std::move(value_)){}

	LemniValue copy() const noexcept override{ return lemniCreateValueAReal(value.handle()); }

	std::size_t numBits() const noexcept override{ return SIZE_MAX; }

	std::string toString() const noexcept override{ return value.toString(); }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	lemni::AReal value;
};

using LemniValueAReal = const LemniValueARealT*;

template<size_t N>
struct LemniBasicValueNat;

template<size_t N>
struct LemniBasicValueInt;

template<size_t N>
struct LemniBasicValueRatio;

template<size_t N>
struct LemniBasicValueReal;

#define DEF_LEMNI_BASIC_VALUE(valTy, bits)\
template<>\
struct LemniBasicValue##valTy<bits>: LemniValue##valTy##T{\
	LemniBasicValue##valTy(Lemni##valTy##bits value_) noexcept\
		: value(value_){}\
	\
	~LemniBasicValue##valTy() = default;\
	\
	LemniValue copy() const noexcept override{ return lemniCreateValue##valTy##bits(value); }\
	\
	std::size_t numBits() const noexcept override{ return sizeof(value) * CHAR_BIT; }\
	\
	std::string toString() const noexcept override{ return to_string(value); }\
	\
	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;\
	\
	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;\
	\
	using Value = Lemni##valTy##bits;\
	\
	Lemni##valTy##bits value;\
};\
using LemniValue##valTy##bits = const LemniBasicValue##valTy<bits>*

DEF_LEMNI_BASIC_VALUE(Nat, 16);
DEF_LEMNI_BASIC_VALUE(Nat, 32);
DEF_LEMNI_BASIC_VALUE(Nat, 64);

DEF_LEMNI_BASIC_VALUE(Int, 16);
DEF_LEMNI_BASIC_VALUE(Int, 32);
DEF_LEMNI_BASIC_VALUE(Int, 64);

DEF_LEMNI_BASIC_VALUE(Ratio, 32);
DEF_LEMNI_BASIC_VALUE(Ratio, 64);
DEF_LEMNI_BASIC_VALUE(Ratio, 128);

DEF_LEMNI_BASIC_VALUE(Real, 32);
DEF_LEMNI_BASIC_VALUE(Real, 64);

#undef DEF_LEMNI_BASIC_VALUE

struct LemniValueStrASCIIT: LemniValueStrT{
	explicit LemniValueStrASCIIT(const LemniStr str): value(lemni::toStdStr(str)){}
	explicit LemniValueStrASCIIT(std::string str): value(std::move(str)){}

	LemniValue copy() const noexcept override{
		LemniStr str{value.c_str(), value.size()};
		return lemniCreateValueStrASCII(str);
	}

	std::size_t numBits() const noexcept override{ return 8; }

	std::string toString() const noexcept override{ return value; }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	std::string value;
};

using LemniValueStrASCII = const LemniValueStrASCIIT*;

struct LemniValueStrUTF8T: LemniValueStrT{
	explicit LemniValueStrUTF8T(const LemniStr str): value(lemni::toStdStr(str)){}
	explicit LemniValueStrUTF8T(std::string str): value(std::move(str)){}

	LemniValue copy() const noexcept override{
		LemniStr str{value.c_str(), value.size()};
		return lemniCreateValueStrUTF8(str);
	}

	std::size_t numBits() const noexcept override{ return 32; }

	std::string toString() const noexcept override{ return value; }

	LemniValue performUnaryOp(const LemniUnaryOp op) const noexcept override;

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	std::string value;
};

using LemniValueStrUTF8 = const LemniValueStrUTF8T*;

struct LemniValueTypeT: LemniValueT{
	explicit LemniValueTypeT(LemniType value_): value(value_){}

	LemniValue copy() const noexcept override{ return lemniCreateValueType(value); }

	std::size_t numBits() const noexcept override{ return 64; }

	std::string toString() const noexcept override{ return lemni::toStdStr(lemniTypeStr(value)); }

	LemniType value;
};

#endif // !LEMNI_LIB_VALUE_HPP
