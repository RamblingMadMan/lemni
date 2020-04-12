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

#include <cstdlib>
#include <climits>

#include <new>
#include <memory>

#include "lemni/Value.h"
#include "lemni/Operator.h"

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
}

struct LemniValueT{
	virtual ~LemniValueT() = default;

	virtual LemniValue copy() const noexcept{ return nullptr; }

	virtual LemniValue deref() const noexcept{ return this; }

	virtual std::size_t numBits() const noexcept = 0;

	virtual std::string toString() const noexcept = 0;

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

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override{ return refed->performBinaryOp(op, rhs); }

	LemniValue refed;
};

using LemniValueRef = const LemniValueRefT*;

struct LemniValueUnitT: LemniValueT{
	LemniValue copy() const noexcept override{ return lemniCreateValueUnit(); }

	std::size_t numBits() const noexcept override{ return 0; }

	std::string toString() const noexcept override{ return "()"; }
};

using LemniValueUnit = const LemniValueUnitT*;

struct LemniValueBoolT: LemniValueT{
	LemniValueBoolT(LemniBool value_) noexcept: value(value_){}

	LemniValue copy() const noexcept override{ return lemniCreateValueBool(value); }

	std::size_t numBits() const noexcept override{ return sizeof(LemniBool) * CHAR_BIT; }

	std::string toString() const noexcept override{ return value ? "true" : "false"; }

	LemniValue performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept override;

	LemniBool value;
};

using LemniValueBool = const LemniValueBoolT*;

struct LemniValueNatT: LemniValueT{};
using LemniValueNat = const LemniValueNatT*;

struct LemniValueIntT: LemniValueT{};
using LemniValueInt = const LemniValueIntT*;

struct LemniValueRatioT: LemniValueT{};
using LemniValueRatio = const LemniValueRatioT*;

struct LemniValueRealT: LemniValueT{};
using LemniValueReal = const LemniValueRealT*;

struct LemniValueANatT: LemniValueNatT{
	LemniValueANatT(LemniAIntConst value_)
		: value(lemni::AInt::from(lemniCreateAIntCopy(value_))){}

	LemniValue copy() const noexcept override{ return lemniCreateValueANat(value.handle()); }

	std::size_t numBits() const noexcept override{ return SIZE_MAX; }

	std::string toString() const noexcept override{ return value.toString(); }

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

LemniValue LemniValueBoolT::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsBool = dynamic_cast<LemniValueBool>(rhs)){
		switch(op){
			case LEMNI_BINARY_AND: return lemni::detail::createLemniValue(value && rhsBool->value);
			case LEMNI_BINARY_OR: return lemni::detail::createLemniValue(value || rhsBool->value);
			case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(value == rhsBool->value);
			case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(value != rhsBool->value);
			default: return nullptr;
		}
	}

	return nullptr;
}

namespace {
	template<typename...>
	struct PromotedT;

#define DEF_PROMOTION(a, b)\
	template<> struct PromotedT<a>{ using Type = b; }

	DEF_PROMOTION(LemniNat16, LemniNat32);
	DEF_PROMOTION(LemniNat32, LemniNat64);
	DEF_PROMOTION(LemniNat64, lemni::AInt);

	DEF_PROMOTION(LemniInt16, LemniInt32);
	DEF_PROMOTION(LemniInt32, LemniInt64);
	DEF_PROMOTION(LemniInt64, lemni::AInt);

	DEF_PROMOTION(LemniReal32, LemniReal64);
	DEF_PROMOTION(LemniReal64, lemni::AReal);

#undef DEF_PROMOTION

	template<typename T>
	using Promoted = typename PromotedT<T>::Type;

	template<size_t LhsN, size_t RhsN>
	LemniValue performOpSizedNatNat(LemniBinaryOp op, const LemniBasicValueNat<LhsN> *lhs, const LemniBasicValueNat<RhsN> *rhs) noexcept{
		using LhsT = LemniBasicValueNat<LhsN>;
		using RhsT = LemniBasicValueNat<RhsN>;

		using LhsPromoted = Promoted<typename LhsT::Value>;
		using RhsPromoted = Promoted<typename RhsT::Value>;

		using LhsSigned = std::make_signed_t<LhsPromoted>;
		using RhsSigned = std::make_signed_t<RhsPromoted>;

		using LhsRatio = lemni::interop::Ratio<LhsN * 4>;
		using RhsRatio = lemni::interop::Ratio<RhsN * 4>;

		switch(op){
			case LEMNI_BINARY_ADD:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsPromoted>::create(RhsPromoted(lhs->value) + RhsPromoted(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsPromoted>::create(LhsPromoted(lhs->value) + LhsPromoted(rhs->value));
				}
			}
			case LEMNI_BINARY_SUB:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsSigned>::create(RhsSigned(lhs->value) - RhsSigned(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsSigned>::create(LhsSigned(lhs->value) - LhsSigned(rhs->value));
				}
			}
			case LEMNI_BINARY_MUL:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsPromoted>::create(RhsPromoted(lhs->value) * RhsPromoted(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsPromoted>::create(LhsPromoted(lhs->value) * LhsPromoted(rhs->value));
				}
			}
			case LEMNI_BINARY_DIV:{
				if constexpr(LhsN < RhsN){

					return lemni::detail::ValueCreator<RhsRatio>::create({RhsSigned(lhs->value), RhsPromoted(rhs->value)});
				}
				else{
					return lemni::detail::ValueCreator<LhsRatio>::create({LhsSigned(lhs->value), LhsPromoted(rhs->value)});
				}
			}
			case LEMNI_BINARY_LT: return lemniCreateValueBool(lhs->value < rhs->value);
			case LEMNI_BINARY_GT: return lemniCreateValueBool(lhs->value > rhs->value);
			case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhs->value <= rhs->value);
			case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhs->value >= rhs->value);
			case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhs->value == rhs->value);
			case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhs->value != rhs->value);
			default: return nullptr;
		}
	}

	template<size_t LhsN, size_t RhsN>
	LemniValue performOpSizedIntInt(LemniBinaryOp op, const LemniBasicValueInt<LhsN> *lhs, const LemniBasicValueInt<RhsN> *rhs) noexcept{
		using LhsT = LemniBasicValueNat<LhsN>;
		using RhsT = LemniBasicValueNat<RhsN>;

		using LhsPromoted = Promoted<typename LhsT::Value>;
		using RhsPromoted = Promoted<typename RhsT::Value>;

		using LhsRatio = lemni::interop::Ratio<LhsN*2>;
		using RhsRatio = lemni::interop::Ratio<RhsN*2>;

		switch(op){
			case LEMNI_BINARY_ADD:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsPromoted>::create(RhsPromoted(lhs->value) + RhsPromoted(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsPromoted>::create(LhsPromoted(lhs->value) + LhsPromoted(rhs->value));
				}
			}
			case LEMNI_BINARY_SUB:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsPromoted>::create(RhsPromoted(lhs->value) - RhsPromoted(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsPromoted>::create(LhsPromoted(lhs->value) - LhsPromoted(rhs->value));
				}
			}
			case LEMNI_BINARY_MUL:{
				if constexpr(LhsN < RhsN){
					return lemni::detail::ValueCreator<RhsPromoted>::create(RhsPromoted(lhs->value) * RhsPromoted(rhs->value));
				}
				else{
					return lemni::detail::ValueCreator<LhsPromoted>::create(LhsPromoted(lhs->value) * LhsPromoted(rhs->value));
				}
			}
			case LEMNI_BINARY_DIV:{
				auto num = lhs->value;
				auto den = rhs->value;

				if(rhs->value < 0){
					num *= -1;
					den *= -1;
				}

				if constexpr(LhsN < RhsN){
					using RhsNat = std::make_unsigned_t<typename RhsT::Value>;
					return lemni::detail::ValueCreator<RhsRatio>::create({lhs->value, RhsNat(rhs->value)});
				}
				else{
					using LhsNat = std::make_unsigned_t<typename LhsT::Value>;
					return lemni::detail::ValueCreator<LhsRatio>::create({lhs->value, LhsNat(rhs->value)});
				}
			}
			case LEMNI_BINARY_LT: return lemniCreateValueBool(lhs->value < rhs->value);
			case LEMNI_BINARY_GT: return lemniCreateValueBool(lhs->value > rhs->value);
			case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhs->value <= rhs->value);
			case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhs->value >= rhs->value);
			case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhs->value == rhs->value);
			case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhs->value != rhs->value);
			default: return nullptr;
		}
	}

	template<size_t LhsN, size_t RhsN>
	LemniValue performOpSizedRatioRatio(LemniBinaryOp op, const LemniBasicValueRatio<LhsN> *lhs, const LemniBasicValueRatio<RhsN> *rhs) noexcept{
		switch(op){
			case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lhs->value + rhs->value);
			case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lhs->value - rhs->value);
			case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lhs->value * rhs->value);
			case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lhs->value / rhs->value);
			case LEMNI_BINARY_LT: return lemniCreateValueBool(lhs->value < rhs->value);
			case LEMNI_BINARY_GT: return lemniCreateValueBool(lhs->value > rhs->value);
			case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhs->value <= rhs->value);
			case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhs->value >= rhs->value);
			case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhs->value == rhs->value);
			case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhs->value != rhs->value);
			default: return nullptr;
		}
	}

	template<size_t LhsN, size_t RhsN>
	LemniValue performOpSizedRealReal(LemniBinaryOp op, const LemniBasicValueReal<LhsN> *lhs, const LemniBasicValueReal<RhsN> *rhs) noexcept{
		using GreaterR = std::conditional_t<(LhsN > RhsN), lemni::interop::Real<LhsN>, lemni::interop::Real<RhsN>>;
		using PromotedR = Promoted<GreaterR>;

		switch(op){
			case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(PromotedR(lhs->value) + PromotedR(rhs->value));
			case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(PromotedR(lhs->value) - PromotedR(rhs->value));
			case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(PromotedR(lhs->value) * PromotedR(rhs->value));
			case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(PromotedR(lhs->value) / PromotedR(rhs->value));
			case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(PromotedR(lhs->value) < PromotedR(rhs->value));
			case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(PromotedR(lhs->value) > PromotedR(rhs->value));
			case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(PromotedR(lhs->value) <= PromotedR(rhs->value));
			case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(PromotedR(lhs->value) >= PromotedR(rhs->value));
			case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(PromotedR(lhs->value) == PromotedR(rhs->value));
			case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(PromotedR(lhs->value) != PromotedR(rhs->value));
			default: return nullptr;
		}
	}

	LemniValue performOpNatNat(LemniBinaryOp op, LemniValueNat lhs, LemniValueNat rhs) noexcept{
		switch(lhs->numBits()){
			case 16:{
				auto lhsN16 = reinterpret_cast<LemniValueNat16>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhs);
						return performOpSizedNatNat(op, lhsN16, rhsN16);
					}
					case 32:{
						auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhs);
						return performOpSizedNatNat(op, lhsN16, rhsN32);
					}
					case 64:{
						auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsN16->value) + lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsN16->value) - lemni::AInt(rhsN64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsN16->value) * lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsN16->value), lemni::AInt(rhsN64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN16->value < rhsN64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN16->value > rhsN64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN16->value <= rhsN64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN16->value >= rhsN64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN16->value == rhsN64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN16->value != rhsN64->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsN = reinterpret_cast<LemniValueANat>(rhs);

						auto lhsN = lemni::AInt(lhsN16->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN + rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN - rhsN->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN * rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN, rhsN->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN < rhsN->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN > rhsN->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN <= rhsN->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN >= rhsN->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN == rhsN->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN != rhsN->value);
							default: return nullptr;
						}
					}
				}
			}
			case 32:{
				auto lhsN32 = reinterpret_cast<LemniValueNat32>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhs);
						return performOpSizedNatNat(op, lhsN32, rhsN16);
					}
					case 32:{
						auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhs);
						return performOpSizedNatNat(op, lhsN32, rhsN32);
					}
					case 64:{
						auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsN32->value) + lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsN32->value) - lemni::AInt(rhsN64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsN32->value) * lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsN32->value), lemni::AInt(rhsN64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN32->value < rhsN64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN32->value > rhsN64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN32->value <= rhsN64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN32->value >= rhsN64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN32->value == rhsN64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN32->value != rhsN64->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsN = reinterpret_cast<LemniValueANat>(rhs);

						auto lhsN = lemni::AInt(lhsN32->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN + rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN - rhsN->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN * rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN, rhsN->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN < rhsN->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN > rhsN->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN <= rhsN->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN >= rhsN->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN == rhsN->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN != rhsN->value);
							default: return nullptr;
						}
					}
				}
			}
			case 64:{
				auto lhsN64 = reinterpret_cast<LemniValueNat64>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsN64->value) + lemni::AInt(rhsN16->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsN64->value) - lemni::AInt(rhsN16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsN64->value) * lemni::AInt(rhsN16->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsN64->value), lemni::AInt(rhsN16->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN64->value < rhsN16->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN64->value > rhsN16->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN64->value <= rhsN16->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN64->value >= rhsN16->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN64->value == rhsN16->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN64->value != rhsN16->value);
							default: return nullptr;
						}
					}
					case 32:{
						auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsN64->value) + lemni::AInt(rhsN32->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsN64->value) - lemni::AInt(rhsN32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsN64->value) * lemni::AInt(rhsN32->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsN64->value), lemni::AInt(rhsN32->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN64->value < rhsN32->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN64->value > rhsN32->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN64->value <= rhsN32->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN64->value >= rhsN32->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN64->value == rhsN32->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN64->value != rhsN32->value);
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsN64->value) + lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsN64->value) - lemni::AInt(rhsN64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsN64->value) * lemni::AInt(rhsN64->value);
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsN64->value), lemni::AInt(rhsN64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN64->value < rhsN64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN64->value > rhsN64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN64->value <= rhsN64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN64->value >= rhsN64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN64->value == rhsN64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN64->value != rhsN64->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsN = reinterpret_cast<LemniValueANat>(rhs);

						auto lhsN = lemni::AInt(lhsN64->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN + rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN - rhsN->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN * rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN, rhsN->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN < rhsN->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN > rhsN->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN <= rhsN->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN >= rhsN->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN == rhsN->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN != rhsN->value);
							default: return nullptr;
						}
					}
				}
			}
			default:{
				auto lhsN = reinterpret_cast<LemniValueANat>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhs);

						auto rhsN = lemni::AInt(rhsN16->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN->value + rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN->value - rhsN;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN->value * rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN->value, rhsN);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN->value < rhsN);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN->value > rhsN);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN->value <= rhsN);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN->value >= rhsN);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN->value == rhsN);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN->value != rhsN);
							default: return nullptr;
						}
					}
					case 32:{
						auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhs);

						auto rhsN = lemni::AInt(rhsN32->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN->value + rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN->value - rhsN;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN->value * rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN->value, rhsN);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN->value < rhsN);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN->value > rhsN);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN->value <= rhsN);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN->value >= rhsN);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN->value == rhsN);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN->value != rhsN);
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhs);

						auto rhsN = lemni::AInt(rhsN64->value);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN->value + rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN->value - rhsN;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN->value * rhsN;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN->value, rhsN);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN->value < rhsN);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN->value > rhsN);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN->value <= rhsN);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN->value >= rhsN);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN->value == rhsN);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN->value != rhsN);
							default: return nullptr;
						}
					}
					default:{
						auto rhsN = reinterpret_cast<LemniValueANat>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsN->value + rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsN->value - rhsN->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsN->value * rhsN->value;
								return lemniCreateValueANat(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsN->value, rhsN->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsN->value < rhsN->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsN->value > rhsN->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsN->value <= rhsN->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsN->value >= rhsN->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsN->value == rhsN->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsN->value != rhsN->value);
							default: return nullptr;
						}
					}
				}

				return nullptr;
			}
		}
	}

	LemniValue performOpIntInt(LemniBinaryOp op, LemniValueInt lhs, LemniValueInt rhs) noexcept{
		switch(lhs->numBits()){
			case 16:{
				auto lhsI16 = reinterpret_cast<LemniValueInt16>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhs);
						return performOpSizedIntInt(op, lhsI16, rhsI16);
					}
					case 32:{
						auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhs);
						return performOpSizedIntInt(op, lhsI16, rhsI32);
					}
					case 64:{
						auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI16->value) + lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI16->value) - lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI16->value) * lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI16->value), lemni::AInt(rhsI64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI16->value < rhsI64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI16->value > rhsI64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI16->value <= rhsI64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI16->value >= rhsI64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI16->value == rhsI64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI16->value != rhsI64->value);
							default: return nullptr;
						}
					}
					default:{}
				}
			}
			case 32:{
				auto lhsI32 = reinterpret_cast<LemniValueInt32>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhs);
						return performOpSizedIntInt(op, lhsI32, rhsI16);
					}
					case 32:{
						auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhs);
						return performOpSizedIntInt(op, lhsI32, rhsI32);
					}
					case 64:{
						auto rhsI64 = reinterpret_cast<LemniValueInt32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI32->value) + lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI32->value) - lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI32->value) * lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI32->value), lemni::AInt(rhsI64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI32->value < rhsI64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI32->value > rhsI64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI32->value <= rhsI64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI32->value >= rhsI64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI32->value == rhsI64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI32->value != rhsI64->value);
							default: return nullptr;
						}
					}
					default:{}
				}
			}
			case 64:{
				auto lhsI64 = reinterpret_cast<LemniValueInt64>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI64->value) + lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI64->value) - lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI64->value) * lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI64->value), lemni::AInt(rhsI16->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI64->value < rhsI16->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI64->value > rhsI16->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI64->value <= rhsI16->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI64->value >= rhsI16->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI64->value == rhsI16->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI64->value != rhsI16->value);
							default: return nullptr;
						}
					}
					case 32:{
						auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI64->value) + lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI64->value) - lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI64->value) * lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI64->value), lemni::AInt(rhsI32->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI64->value < rhsI32->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI64->value > rhsI32->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI64->value <= rhsI32->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI64->value >= rhsI32->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI64->value == rhsI32->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI64->value != rhsI32->value);
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI64->value) + lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI64->value) - lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI64->value) * lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI64->value), lemni::AInt(rhsI64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI64->value < rhsI64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI64->value > rhsI64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI64->value <= rhsI64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI64->value >= rhsI64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI64->value == rhsI64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI64->value != rhsI64->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsI = reinterpret_cast<LemniValueAInt>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::AInt(lhsI64->value) + rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::AInt(lhsI64->value) - rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::AInt(lhsI64->value) * rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lemni::AInt(lhsI64->value), rhsI->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lemni::AInt(lhsI64->value) < rhsI->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lemni::AInt(lhsI64->value) > rhsI->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lemni::AInt(lhsI64->value) <= rhsI->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lemni::AInt(lhsI64->value) >= rhsI->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lemni::AInt(lhsI64->value) == rhsI->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lemni::AInt(lhsI64->value) != rhsI->value);
							default: return nullptr;
						}
					}
				}
			}
			default:{
				auto lhsI = reinterpret_cast<LemniValueAInt>(lhs);

				switch(rhs->numBits()){
					case 16:{
						auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsI->value + lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsI->value - lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsI->value * lemni::AInt(rhsI16->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsI->value, lemni::AInt(rhsI16->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI->value < lemni::AInt(rhsI16->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI->value > lemni::AInt(rhsI16->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI->value <= lemni::AInt(rhsI16->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI->value >= lemni::AInt(rhsI16->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI->value == lemni::AInt(rhsI16->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI->value != lemni::AInt(rhsI16->value));
							default: return nullptr;
						}
					}
					case 32:{
						auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsI->value + lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsI->value - lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsI->value * lemni::AInt(rhsI32->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsI->value, lemni::AInt(rhsI32->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI->value < lemni::AInt(rhsI32->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI->value > lemni::AInt(rhsI32->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI->value <= lemni::AInt(rhsI32->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI->value >= lemni::AInt(rhsI32->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI->value == lemni::AInt(rhsI32->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI->value != lemni::AInt(rhsI32->value));
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsI->value + lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsI->value - lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsI->value * lemni::AInt(rhsI64->value);
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsI->value, lemni::AInt(rhsI64->value));
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI->value < lemni::AInt(rhsI64->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI->value > lemni::AInt(rhsI64->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI->value <= lemni::AInt(rhsI64->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI->value >= lemni::AInt(rhsI64->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI->value == lemni::AInt(rhsI64->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI->value != lemni::AInt(rhsI64->value));
							default: return nullptr;
						}
					}
					default:{
						auto rhsI = reinterpret_cast<LemniValueAInt>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsI->value + rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsI->value - rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsI->value * rhsI->value;
								return lemniCreateValueAInt(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsI->value, rhsI->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsI->value < rhsI->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsI->value > rhsI->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsI->value <= rhsI->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsI->value >= rhsI->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsI->value == rhsI->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsI->value != rhsI->value);
							default: return nullptr;
						}
					}
				}
			}
		}
	}

	LemniValue performOpRatioRatio(LemniBinaryOp op, LemniValueRatio lhs, LemniValueRatio rhs) noexcept{
		switch(lhs->numBits()){
			case 32:{
				auto lhsQ32 = reinterpret_cast<LemniValueRatio32>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhs);
						return performOpSizedRatioRatio(op, lhsQ32, rhsQ32);
					}
					case 64:{
						auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhs);
						return performOpSizedRatioRatio(op, lhsQ32, rhsQ64);
					}
					case 128:{
						auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ32->value) + lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ32->value) - lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ32->value) * lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ32->value) / lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ32->value < rhsQ128->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ32->value > rhsQ128->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ32->value <= rhsQ128->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ32->value >= rhsQ128->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ32->value == rhsQ128->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ32->value != rhsQ128->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsQ = reinterpret_cast<LemniValueARatio>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ32->value) + rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ32->value) - rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ32->value) * rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ32->value) / rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) < rhsQ->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) > rhsQ->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) <= rhsQ->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) >= rhsQ->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) == rhsQ->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ32->value) != rhsQ->value);
							default: return nullptr;
						}
					}
				}
			}
			case 64:{
				auto lhsQ64 = reinterpret_cast<LemniValueRatio64>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhs);
						return performOpSizedRatioRatio(op, lhsQ64, rhsQ32);
					}
					case 64:{
						auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhs);
						return performOpSizedRatioRatio(op, lhsQ64, rhsQ64);
					}
					case 128:{
						auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ64->value) + lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ64->value) - lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ64->value) * lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ64->value) / lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ64->value < rhsQ128->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ64->value > rhsQ128->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ64->value <= rhsQ128->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ64->value >= rhsQ128->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ64->value == rhsQ128->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ64->value != rhsQ128->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsQ = reinterpret_cast<LemniValueARatio>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ64->value) + rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ64->value) - rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ64->value) * rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ64->value) / rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) < rhsQ->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) > rhsQ->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) <= rhsQ->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) >= rhsQ->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) == rhsQ->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ64->value) != rhsQ->value);
							default: return nullptr;
						}
					}
				}
			}
			case 128:{
				auto lhsQ128 = reinterpret_cast<LemniValueRatio128>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ128->value) + lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ128->value) - lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ128->value) * lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ128->value) / lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ128->value < rhsQ32->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ128->value > rhsQ32->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ128->value <= rhsQ32->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ128->value >= rhsQ32->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ128->value == rhsQ32->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ128->value != rhsQ32->value);
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ128->value) + lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ128->value) - lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ128->value) * lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ128->value) / lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ128->value < rhsQ64->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ128->value > rhsQ64->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ128->value <= rhsQ64->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ128->value >= rhsQ64->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ128->value == rhsQ64->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ128->value != rhsQ64->value);
							default: return nullptr;
						}
					}
					case 128:{
						auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ128->value) + lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ128->value) - lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ128->value) * lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ128->value) / lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ128->value < rhsQ128->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ128->value > rhsQ128->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ128->value <= rhsQ128->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ128->value >= rhsQ128->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ128->value == rhsQ128->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ128->value != rhsQ128->value);
							default: return nullptr;
						}
					}
					default:{
						auto rhsQ = reinterpret_cast<LemniValueARatio>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lemni::ARatio(lhsQ128->value) + rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lemni::ARatio(lhsQ128->value) - rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lemni::ARatio(lhsQ128->value) * rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lemni::ARatio(lhsQ128->value) / rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) < rhsQ->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) > rhsQ->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) <= rhsQ->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) >= rhsQ->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) == rhsQ->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lemni::ARatio(lhsQ128->value) != rhsQ->value);
							default: return nullptr;
						}
					}
				}
			}
			default:{
				auto lhsQ = reinterpret_cast<LemniValueARatio>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsQ->value + lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsQ->value - lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsQ->value * lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lhsQ->value / lemni::ARatio(rhsQ32->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ->value < lemni::ARatio(rhsQ32->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ->value > lemni::ARatio(rhsQ32->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ->value <= lemni::ARatio(rhsQ32->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ->value >= lemni::ARatio(rhsQ32->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ->value == lemni::ARatio(rhsQ32->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ->value != lemni::ARatio(rhsQ32->value));
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsQ->value + lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsQ->value - lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsQ->value * lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lhsQ->value / lemni::ARatio(rhsQ64->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ->value < lemni::ARatio(rhsQ64->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ->value > lemni::ARatio(rhsQ64->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ->value <= lemni::ARatio(rhsQ64->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ->value >= lemni::ARatio(rhsQ64->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ->value == lemni::ARatio(rhsQ64->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ->value != lemni::ARatio(rhsQ64->value));
							default: return nullptr;
						}
					}
					case 128:{
						auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsQ->value + lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsQ->value - lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsQ->value * lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lhsQ->value / lemni::ARatio(rhsQ128->value);
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ->value < lemni::ARatio(rhsQ128->value));
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ->value > lemni::ARatio(rhsQ128->value));
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ->value <= lemni::ARatio(rhsQ128->value));
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ->value >= lemni::ARatio(rhsQ128->value));
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ->value == lemni::ARatio(rhsQ128->value));
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ->value != lemni::ARatio(rhsQ128->value));
							default: return nullptr;
						}
					}
					default:{
						auto rhsQ = reinterpret_cast<LemniValueARatio>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD:{
								auto res = lhsQ->value + rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_SUB:{
								auto res = lhsQ->value - rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_MUL:{
								auto res = lhsQ->value * rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_DIV:{
								auto res = lhsQ->value / rhsQ->value;
								return lemniCreateValueARatio(res.handle());
							}
							case LEMNI_BINARY_LT: return lemniCreateValueBool(lhsQ->value < rhsQ->value);
							case LEMNI_BINARY_GT: return lemniCreateValueBool(lhsQ->value > rhsQ->value);
							case LEMNI_BINARY_LTEQ: return lemniCreateValueBool(lhsQ->value <= rhsQ->value);
							case LEMNI_BINARY_GTEQ: return lemniCreateValueBool(lhsQ->value >= rhsQ->value);
							case LEMNI_BINARY_EQ: return lemniCreateValueBool(lhsQ->value == rhsQ->value);
							case LEMNI_BINARY_NEQ: return lemniCreateValueBool(lhsQ->value != rhsQ->value);
							default: return nullptr;
						}
					}
				}
			}
		}
	}

	LemniValue performOpRealReal(LemniBinaryOp op, LemniValueReal lhs, LemniValueReal rhs) noexcept{
		switch(lhs->numBits()){
			case 32:{
				auto lhsR32 = reinterpret_cast<LemniValueReal32>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsR32 = reinterpret_cast<LemniValueReal32>(rhs);
						return performOpSizedRealReal(op, lhsR32, rhsR32);
					}
					case 64:{
						auto rhsR64 = reinterpret_cast<LemniValueReal64>(rhs);
						return performOpSizedRealReal(op, lhsR32, rhsR64);
					}
					default:{
						auto rhsR = reinterpret_cast<LemniValueAReal>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) + rhsR->value);
							case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) - rhsR->value);
							case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) * rhsR->value);
							case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) / rhsR->value);
							case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) < rhsR->value);
							case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) > rhsR->value);
							case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) <= rhsR->value);
							case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) >= rhsR->value);
							case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) == rhsR->value);
							case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR32->value) != rhsR->value);
							default: return nullptr;
						}
					}
				}
			}
			case 64:{
				auto lhsR64 = reinterpret_cast<LemniValueReal64>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsR32 = reinterpret_cast<LemniValueReal32>(rhs);
						return performOpSizedRealReal(op, lhsR64, rhsR32);
					}
					case 64:{
						auto rhsR64 = reinterpret_cast<LemniValueReal64>(rhs);
						return performOpSizedRealReal(op, lhsR64, rhsR64);
					}
					default:{
						auto rhsR = reinterpret_cast<LemniValueAReal>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) + rhsR->value);
							case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) - rhsR->value);
							case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) * rhsR->value);
							case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) / rhsR->value);
							case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) < rhsR->value);
							case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) > rhsR->value);
							case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) <= rhsR->value);
							case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) >= rhsR->value);
							case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) == rhsR->value);
							case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(lemni::AReal(lhsR64->value) != rhsR->value);
							default: return nullptr;
						}
					}
				}
			}
			default:{
				auto lhsR = reinterpret_cast<LemniValueAReal>(lhs);

				switch(rhs->numBits()){
					case 32:{
						auto rhsR32 = reinterpret_cast<LemniValueReal32>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lhsR->value + lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lhsR->value - lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lhsR->value * lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lhsR->value / lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(lhsR->value < lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(lhsR->value > lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(lhsR->value <= lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(lhsR->value >= lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(lhsR->value == lemni::AReal(rhsR32->value));
							case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(lhsR->value != lemni::AReal(rhsR32->value));
							default: return nullptr;
						}
					}
					case 64:{
						auto rhsR64 = reinterpret_cast<LemniValueReal64>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lhsR->value + lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lhsR->value - lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lhsR->value * lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lhsR->value / lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(lhsR->value < lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(lhsR->value > lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(lhsR->value <= lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(lhsR->value >= lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(lhsR->value == lemni::AReal(rhsR64->value));
							case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(lhsR->value != lemni::AReal(rhsR64->value));
							default: return nullptr;
						}
					}
					default:{
						auto rhsR = reinterpret_cast<LemniValueAReal>(rhs);

						switch(op){
							case LEMNI_BINARY_ADD: return lemni::detail::createLemniValue(lhsR->value + rhsR->value);
							case LEMNI_BINARY_SUB: return lemni::detail::createLemniValue(lhsR->value - rhsR->value);
							case LEMNI_BINARY_MUL: return lemni::detail::createLemniValue(lhsR->value * rhsR->value);
							case LEMNI_BINARY_DIV: return lemni::detail::createLemniValue(lhsR->value / rhsR->value);
							case LEMNI_BINARY_LT: return lemni::detail::createLemniValue(lhsR->value < rhsR->value);
							case LEMNI_BINARY_GT: return lemni::detail::createLemniValue(lhsR->value > rhsR->value);
							case LEMNI_BINARY_LTEQ: return lemni::detail::createLemniValue(lhsR->value <= rhsR->value);
							case LEMNI_BINARY_GTEQ: return lemni::detail::createLemniValue(lhsR->value >= rhsR->value);
							case LEMNI_BINARY_EQ: return lemni::detail::createLemniValue(lhsR->value == rhsR->value);
							case LEMNI_BINARY_NEQ: return lemni::detail::createLemniValue(lhsR->value != rhsR->value);
							default: return nullptr;
						}
					}
				}
			}
		}
	}
}

LemniValue LemniBasicValueNat<16>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		return performOpNatNat(op, this, rhsNat);
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		LemniBasicValueInt<32> lhs(value);
		return performOpIntInt(op, &lhs, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniBasicValueRatio<64> lhsQ64({value, 1});
		return performOpRatioRatio(op, &lhsQ64, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<32> lhsR32(value);
		return performOpRealReal(op, &lhsR32, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueNat<32>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		return performOpNatNat(op, this, rhsNat);
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		LemniBasicValueInt<64> lhs(value);
		return performOpIntInt(op, &lhs, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniBasicValueRatio<128> lhsQ128({value, 1});
		return performOpRatioRatio(op, &lhsQ128, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<64> lhsR64(value);
		return performOpRealReal(op, &lhsR64, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueNat<64>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		return performOpNatNat(op, this, rhsNat);
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		auto lhs = LemniValueAIntT(lemni::AInt(value));
		return performOpIntInt(op, &lhs, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniValueARatioT lhsQ(lemni::ARatio(value, 1));
		return performOpRatioRatio(op, &lhsQ, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniValueARealT lhsR(lemni::AReal{value});
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniValueANatT::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		return performOpNatNat(op, this, rhsNat);
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		auto lhsI = LemniValueAIntT(value);
		return performOpIntInt(op, &lhsI, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		auto lhsQ = LemniValueARatioT(lemni::ARatio(value, lemni::AInt(1)));
		return performOpRatioRatio(op, &lhsQ, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		auto lhsR = LemniValueARealT(lemni::AReal(value));
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueInt<16>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueInt<32> rhsI32(rhsN16->value);
				return performOpIntInt(op, this, &rhsI32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueInt<64> rhsI64(rhsN32->value);
				return performOpIntInt(op, this, &rhsI64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				LemniValueAIntT rhsI(lemni::AInt(rhsN64->value));
				return performOpIntInt(op, this, &rhsI);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				LemniValueAIntT rhsI(rhsN->value);
				return performOpIntInt(op, this, &rhsI);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		return performOpIntInt(op, this, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniBasicValueRatio<32> lhsQ32({value, 1});
		return performOpRatioRatio(op, &lhsQ32, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<32> lhsR32(value);
		return performOpRealReal(op, &lhsR32, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueInt<32>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueInt<32> rhsI32(rhsN16->value);
				return performOpIntInt(op, this, &rhsI32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueInt<64> rhsI64(rhsN32->value);
				return performOpIntInt(op, this, &rhsI64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				LemniValueAIntT rhsI(lemni::AInt(rhsN64->value));
				return performOpIntInt(op, this, &rhsI);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				LemniValueAIntT rhsI(rhsN->value);
				return performOpIntInt(op, this, &rhsI);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		return performOpIntInt(op, this, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniBasicValueRatio<64> lhsQ64({value, 1});
		return performOpRatioRatio(op, &lhsQ64, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<64> lhsR64(value);
		return performOpRealReal(op, &lhsR64, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueInt<64>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueInt<32> rhsI32(rhsN16->value);
				return performOpIntInt(op, this, &rhsI32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueInt<64> rhsI64(rhsN32->value);
				return performOpIntInt(op, this, &rhsI64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				LemniValueAIntT rhsI(lemni::AInt(rhsN64->value));
				return performOpIntInt(op, this, &rhsI);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				LemniValueAIntT rhsI(rhsN->value);
				return performOpIntInt(op, this, &rhsI);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		return performOpIntInt(op, this, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		LemniBasicValueRatio<128> lhsQ128({value, 1});
		return performOpRatioRatio(op, &lhsQ128, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniValueARealT lhsR(lemni::AReal{value});
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniValueAIntT::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueInt<32> rhsI32(rhsN16->value);
				return performOpIntInt(op, this, &rhsI32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueInt<64> rhsI64(rhsN32->value);
				return performOpIntInt(op, this, &rhsI64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				LemniValueAIntT rhsI(lemni::AInt(rhsN64->value));
				return performOpIntInt(op, this, &rhsI);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				LemniValueAIntT rhsI(rhsN->value);
				return performOpIntInt(op, this, &rhsI);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		return performOpIntInt(op, this, rhsInt);
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		auto lhsQ = LemniValueARatioT(lemni::ARatio(value, lemni::AInt(1)));
		return performOpRatioRatio(op, &lhsQ, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		auto lhsR = LemniValueARealT(lemni::AReal{value});
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueRatio<32>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsN16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsN32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(lemni::AInt(rhsN64->value), lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsNat);
				LemniBasicValueRatio<32> rhsQ32({rhsI16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsI32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsI64->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		return performOpRatioRatio(op, this, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<32> lhsR32(float(value.num) / value.den);
		return performOpRealReal(op, &lhsR32, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueRatio<64>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsN16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsN32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(lemni::AInt(rhsN64->value), lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsNat);
				LemniBasicValueRatio<32> rhsQ32({rhsI16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsI32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsI64->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		return performOpRatioRatio(op, this, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniBasicValueReal<64> lhsR64(float(value.num) / value.den);
		return performOpRealReal(op, &lhsR64, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueRatio<128>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsN16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsN32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(lemni::AInt(rhsN64->value), lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsNat);
				LemniBasicValueRatio<32> rhsQ32({rhsI16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsI32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsI64->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		return performOpRatioRatio(op, this, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		LemniValueARealT lhsR(lemni::AReal(value.num) / lemni::AReal(value.den));
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniValueARatioT::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsN16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsN32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(lemni::AInt(rhsN64->value), lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsNat);
				LemniBasicValueRatio<32> rhsQ32({rhsI16->value, 1});
				return performOpRatioRatio(op, this, &rhsQ32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsNat);
				LemniBasicValueRatio<64> rhsQ64({rhsI32->value, 1});
				return performOpRatioRatio(op, this, &rhsQ64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsNat);
				LemniBasicValueRatio<128> rhsQ128({rhsI64->value, 1});
				return performOpRatioRatio(op, this, &rhsQ128);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsQ = LemniValueARatioT(lemni::ARatio(rhsN->value, lemni::AInt(1)));
				return performOpRatioRatio(op, this, &rhsQ);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		return performOpRatioRatio(op, this, rhsRatio);
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		auto lhsR = LemniValueARealT(lemni::AReal(value));
		return performOpRealReal(op, &lhsR, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueReal<32>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				auto rhsR32 = LemniBasicValueReal<32>(rhsN16->value);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				auto rhsR64 = LemniBasicValueReal<64>(rhsN32->value);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsInt);
				auto rhsR32 = LemniBasicValueReal<32>(rhsI16->value);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsInt);
				auto rhsR64 = LemniBasicValueReal<64>(rhsI32->value);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsI = reinterpret_cast<LemniValueAInt>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		switch(rhsRatio->numBits()){
			case 32:{
				auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhsRatio);
				auto rhsR32 = LemniBasicValueReal<32>(float(rhsQ32->value.num) / rhsQ32->value.den);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 64:{
				auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhsRatio);
				auto rhsR64 = LemniBasicValueReal<64>(float(rhsQ64->value.num) / rhsQ64->value.den);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 128:{
				auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(lemni::ARatio(rhsQ128->value.num, rhsQ128->value.den)));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsQ = reinterpret_cast<LemniValueARatio>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsQ->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		return performOpRealReal(op, this, rhsReal);
	}

	return nullptr;
}

LemniValue LemniBasicValueReal<64>::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				auto rhsR32 = LemniBasicValueReal<32>(rhsN16->value);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				auto rhsR64 = LemniBasicValueReal<64>(rhsN32->value);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsInt);
				auto rhsR32 = LemniBasicValueReal<32>(rhsI16->value);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsInt);
				auto rhsR64 = LemniBasicValueReal<64>(rhsI32->value);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsI = reinterpret_cast<LemniValueAInt>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		switch(rhsRatio->numBits()){
			case 32:{
				auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhsRatio);
				auto rhsR32 = LemniBasicValueReal<32>(float(rhsQ32->value.num) / rhsQ32->value.den);
				return performOpRealReal(op, this, &rhsR32);
			}
			case 64:{
				auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhsRatio);
				auto rhsR64 = LemniBasicValueReal<64>(float(rhsQ64->value.num) / rhsQ64->value.den);
				return performOpRealReal(op, this, &rhsR64);
			}
			case 128:{
				auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(lemni::ARatio(rhsQ128->value.num, rhsQ128->value.den)));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsQ = reinterpret_cast<LemniValueARatio>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsQ->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		return performOpRealReal(op, this, rhsReal);
	}

	return nullptr;
}

LemniValue LemniValueARealT::performBinaryOp(const LemniBinaryOp op, const LemniValue rhs) const noexcept{
	if(auto rhsNat = dynamic_cast<LemniValueNat>(rhs)){
		switch(rhsNat->numBits()){
			case 16:{
				auto rhsN16 = reinterpret_cast<LemniValueNat16>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN16->value));
				return performOpRealReal(op, this, &rhsR);
			}
			case 32:{
				auto rhsN32 = reinterpret_cast<LemniValueNat32>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN32->value));
				return performOpRealReal(op, this, &rhsR);
			}
			case 64:{
				auto rhsN64 = reinterpret_cast<LemniValueNat64>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsN = reinterpret_cast<LemniValueANat>(rhsNat);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsN->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsInt = dynamic_cast<LemniValueInt>(rhs)){
		switch(rhsInt->numBits()){
			case 16:{
				auto rhsI16 = reinterpret_cast<LemniValueInt16>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI16->value));
				return performOpRealReal(op, this, &rhsR);
			}
			case 32:{
				auto rhsI32 = reinterpret_cast<LemniValueInt32>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI32->value));
				return performOpRealReal(op, this, &rhsR);
			}
			case 64:{
				auto rhsI64 = reinterpret_cast<LemniValueInt64>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI64->value));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsI = reinterpret_cast<LemniValueAInt>(rhsInt);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsI->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsRatio = dynamic_cast<LemniValueRatio>(rhs)){
		switch(rhsRatio->numBits()){
			case 32:{
				auto rhsQ32 = reinterpret_cast<LemniValueRatio32>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(lemni::ARatio(long(rhsQ32->value.num), rhsQ32->value.den)));
				return performOpRealReal(op, this, &rhsR);
			}
			case 64:{
				auto rhsQ64 = reinterpret_cast<LemniValueRatio64>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(lemni::ARatio(long(rhsQ64->value.num), rhsQ64->value.den)));
				return performOpRealReal(op, this, &rhsR);
			}
			case 128:{
				auto rhsQ128 = reinterpret_cast<LemniValueRatio128>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(lemni::ARatio(rhsQ128->value.num, rhsQ128->value.den)));
				return performOpRealReal(op, this, &rhsR);
			}
			default:{
				auto rhsQ = reinterpret_cast<LemniValueARatio>(rhsRatio);
				auto rhsR = LemniValueARealT(lemni::AReal(rhsQ->value));
				return performOpRealReal(op, this, &rhsR);
			}
		}
	}
	else if(auto rhsReal = dynamic_cast<LemniValueReal>(rhs)){
		return performOpRealReal(op, this, rhsReal);
	}

	return nullptr;
}

void lemniDestroyValue(LemniValue value){
	std::destroy_at(value);
	std::free(const_cast<void*>(reinterpret_cast<const void*>(value)));
}

namespace {
	template<typename T, typename ... Args>
	auto allocValue(Args &&... args){
		auto mem = std::malloc(sizeof(T));
		return new(mem) T(std::forward<Args>(args)...);
	}
}

LemniValue lemniCreateValueCopy(LemniValue value){ return value->copy(); }

LemniValue lemniCreateValueRef(LemniValue value){ return allocValue<LemniValueRefT>(value->deref()); }

LemniValue lemniCreateValueUnit(void){ return allocValue<LemniValueUnitT>(); }

LemniValue lemniCreateValueBool(const LemniBool b){ return allocValue<LemniValueBoolT>(b); }

LemniValue lemniCreateValueNat16(const LemniNat16 n16){ return allocValue<LemniBasicValueNat<16>>(n16); }
LemniValue lemniCreateValueNat32(const LemniNat32 n32){ return allocValue<LemniBasicValueNat<32>>(n32); }
LemniValue lemniCreateValueNat64(const LemniNat64 n64){ return allocValue<LemniBasicValueNat<64>>(n64); }
LemniValue lemniCreateValueANat(LemniAIntConst aint){ return allocValue<LemniValueANatT>(aint); }

LemniValue lemniCreateValueInt16(const LemniInt16 i16){ return allocValue<LemniBasicValueInt<16>>(i16); }
LemniValue lemniCreateValueInt32(const LemniInt32 i32){ return allocValue<LemniBasicValueInt<32>>(i32); }
LemniValue lemniCreateValueInt64(const LemniInt64 i64){ return allocValue<LemniBasicValueInt<16>>(i64); }
LemniValue lemniCreateValueAInt(LemniAIntConst aint){ return allocValue<LemniValueAIntT>(aint); }

LemniValue lemniCreateValueRatio32(const LemniRatio32 q32){ return allocValue<LemniBasicValueRatio<32>>(q32); }
LemniValue lemniCreateValueRatio64(const LemniRatio64 q64){ return allocValue<LemniBasicValueRatio<64>>(q64); }
LemniValue lemniCreateValueRatio128(const LemniRatio128 q128){ return allocValue<LemniBasicValueRatio<128>>(q128); }
LemniValue lemniCreateValueARatio(LemniARatioConst aratio){ return allocValue<LemniValueARatioT>(aratio); }

LemniValue lemniCreateValueReal32(const LemniReal32 r32){ return allocValue<LemniBasicValueReal<32>>(r32); }
LemniValue lemniCreateValueReal64(const LemniReal64 r64){ return allocValue<LemniBasicValueReal<64>>(r64); }
LemniValue lemniCreateValueAReal(LemniARealConst areal){ return allocValue<LemniValueARealT>(areal); }

void lemniValueStr(LemniValue value, void *user, LemniValueStrCB cb){
	auto str = value->toString();
	cb(user, {str.c_str(), str.size()});
}

LemniValue lemniValueBinaryOp(LemniBinaryOp op, LemniValue lhs, LemniValue rhs){
	return lhs->performBinaryOp(op, rhs->deref());
}

LemniValue lemniValueAdd(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_ADD, lhs, rhs); }
LemniValue lemniValueSub(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_SUB, lhs, rhs); }
LemniValue lemniValueMul(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_MUL, lhs, rhs); }
LemniValue lemniValueDiv(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_DIV, lhs, rhs); }

LemniValue lemniValueAnd(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_AND, lhs, rhs); }
LemniValue lemniValueOr(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_OR, lhs, rhs); }

LemniValue lemniValueLessThan(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_LT, lhs, rhs); }
LemniValue lemniValueGreaterThan(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_GT, lhs, rhs); }
LemniValue lemniValueLessThanEqual(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_LTEQ, lhs, rhs); }
LemniValue lemniValueGreaterThanEqual(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_GTEQ, lhs, rhs); }
LemniValue lemniValueEqual(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_EQ, lhs, rhs); }
LemniValue lemniValueNotEqual(LemniValue lhs, LemniValue rhs){ return lemniValueBinaryOp(LEMNI_BINARY_NEQ, lhs, rhs); }
