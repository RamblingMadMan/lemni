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

#ifndef LEMNI_LIB_TYPE_HPP
#define LEMNI_LIB_TYPE_HPP 1

#include <cstring>

#include <vector>
#include <string>

using namespace std::string_literals;

#include "lemni/Str.h"
#include "lemni/Type.h"
#include "lemni/Operator.h"

template<size_t N>
static inline constexpr uint32_t binaryOpFlags(const LemniBinaryOp (&ops)[N]) noexcept{
	uint32_t flags = 0;
	for(size_t i = 0; i < N; i++){
		flags |= ops[i];
	}

	return flags;
}

template<size_t N>
static inline constexpr uint32_t unaryOpFlags(const LemniUnaryOp (&ops)[N]) noexcept{
	uint32_t flags = 0;

	for(size_t i = 0; i < N; i++){
		flags |= ops[i];
	}

	return flags;
}

static inline constexpr auto zeroedTypeInfo() noexcept{
	auto info = LemniTypeInfo{
		.binaryOpFlags = 0,
		.unaryOpFlags = 0,
		.typeClass = LEMNI_TYPECLASS_EMPTY
	};

	for(std::size_t i = 0; i < sizeof(info.info.bytes); i++){
		info.info.bytes[i] = 0;
	}

	return info;
}

static inline constexpr auto zeroPadTypeInfo(const LemniTypeInfo info) noexcept{
	auto ret = zeroedTypeInfo();
	ret.binaryOpFlags = info.binaryOpFlags;
	ret.unaryOpFlags = info.unaryOpFlags;
	ret.typeClass = info.typeClass;

	switch(info.typeClass & ~LEMNI_TYPECLASS_PSEUDO){
		case LEMNI_TYPECLASS_SCALAR:{
			ret.info.scalar.traits = info.info.scalar.traits;
			ret.info.scalar.numBits = info.info.scalar.numBits;
			break;
		}

		case LEMNI_TYPECLASS_SUM:{
			ret.info.sum.numCases = info.info.sum.numCases;
			ret.info.sum.caseTypeIndices = info.info.sum.caseTypeIndices;
			break;
		}

		case LEMNI_TYPECLASS_PRODUCT:{
			ret.info.product.numElems = info.info.product.numElems;
			ret.info.product.elemTypeIndices = info.info.product.elemTypeIndices;
			break;
		}

		case LEMNI_TYPECLASS_SIGMA:{
			ret.info.sigma.numElems = info.info.sigma.numElems;
			ret.info.sigma.elemTypeIdx = info.info.sigma.elemTypeIdx;
			break;
		}

		case LEMNI_TYPECLASS_RECORD:{
			ret.info.record.numFields = info.info.record.numFields;
			ret.info.record.fieldNameIndices = info.info.record.fieldNameIndices;
			ret.info.record.fieldTypeIndices = info.info.record.fieldTypeIndices;
			break;
		}

		case LEMNI_TYPECLASS_CALLABLE:{
			ret.info.callable.numClosed = info.info.callable.numClosed;
			ret.info.callable.numParams = info.info.callable.numParams;
			ret.info.callable.resultTypeIdx = info.info.callable.resultTypeIdx;
			ret.info.callable.paramTypeIndices = info.info.callable.paramTypeIndices;
			ret.info.callable.closedTypeIndices = info.info.callable.closedTypeIndices;
			break;
		}

		case LEMNI_TYPECLASS_TOP:
		case LEMNI_TYPECLASS_BOTTOM:
		case LEMNI_TYPECLASS_META:
		case LEMNI_TYPECLASS_EMPTY:
		default:
			break;
	}

	return ret;
}

static inline constexpr auto topTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_TOP;
	return ret;
}

static inline constexpr auto bottomTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_BOTTOM;
	return ret;
}

static inline constexpr auto metaTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_META;
	return ret;
}

static inline constexpr auto exprTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_EXPR;
	return ret;
}

static inline constexpr auto scalarTypeInfo(const uint32_t traits, const uint32_t numBits = 0) noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_SCALAR;
	ret.info.scalar.numBits = numBits;
	ret.info.scalar.traits = traits;
	return ret;
}

static inline constexpr auto sumTypeInfo(const uint64_t numCases, const uint64_t *const caseTypeIndices) noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_SUM;
	ret.info.sum.numCases = numCases;
	ret.info.sum.caseTypeIndices = caseTypeIndices;
	return ret;
}

static inline constexpr auto productTypeInfo(const uint64_t numElems, const uint64_t *const elemTypeIndices) noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_PRODUCT;
	ret.info.product.numElems = numElems;
	ret.info.product.elemTypeIndices = elemTypeIndices;
	return ret;
}

static inline constexpr auto recordTypeInfo(
	const uint64_t numFields, const uint64_t *const fieldTypeIndices, const uint64_t *const fieldNameIndices
) noexcept
{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_RECORD;
	ret.info.record.numFields = numFields;
	ret.info.record.fieldTypeIndices = fieldTypeIndices;
	ret.info.record.fieldNameIndices = fieldNameIndices;
	return ret;
}

static inline constexpr auto unitTypeInfo() noexcept{
	return scalarTypeInfo(LEMNI_SCALAR_UNIT);
}

static inline constexpr auto boolTypeInfo() noexcept{
	constexpr LemniBinaryOp binaryOps[] = {
		LEMNI_BINARY_EQ, LEMNI_BINARY_NEQ
	};

	constexpr LemniUnaryOp unaryOps[] = {
		LEMNI_UNARY_NOT
	};

	auto ret = scalarTypeInfo(LEMNI_SCALAR_RANGE | LEMNI_SCALAR_BOOL, 1);
	ret.binaryOpFlags = binaryOpFlags(binaryOps);
	ret.unaryOpFlags = unaryOpFlags(unaryOps);
	return ret;
}

static inline constexpr auto numberTypeInfo() noexcept{
	constexpr LemniBinaryOp binaryOps[] = {
		LEMNI_BINARY_ADD, LEMNI_BINARY_SUB,
		LEMNI_BINARY_MUL, LEMNI_BINARY_DIV,
		LEMNI_BINARY_MOD,
		LEMNI_BINARY_POW,
		LEMNI_BINARY_EQ, LEMNI_BINARY_NEQ,
		LEMNI_BINARY_LT, LEMNI_BINARY_GT,
		LEMNI_BINARY_LTEQ, LEMNI_BINARY_GTEQ
	};

	constexpr LemniUnaryOp unaryOps[] = {
		LEMNI_UNARY_NEG
	};

	auto ret = scalarTypeInfo(LEMNI_SCALAR_RANGE);
	ret.binaryOpFlags = binaryOpFlags(binaryOps);
	ret.unaryOpFlags = unaryOpFlags(unaryOps);
	return ret;
}

static inline constexpr auto natTypeInfo(const uint32_t numBits = 0) noexcept{
	auto ret = numberTypeInfo();
	ret.info.scalar.numBits = numBits;
	ret.info.scalar.traits = LEMNI_SCALAR_RANGE | LEMNI_SCALAR_NAT | LEMNI_SCALAR_INT | LEMNI_SCALAR_RATIO | LEMNI_SCALAR_REAL;
	return ret;
}

static inline constexpr auto intTypeInfo(const uint32_t numBits = 0) noexcept{
	auto ret = numberTypeInfo();
	ret.info.scalar.numBits = numBits;
	ret.info.scalar.traits = LEMNI_SCALAR_RANGE | LEMNI_SCALAR_INT | LEMNI_SCALAR_RATIO | LEMNI_SCALAR_REAL;
	return ret;
}

static inline constexpr auto ratioTypeInfo(const uint32_t numBits = 0) noexcept{
	auto ret = numberTypeInfo();
	ret.info.scalar.numBits = numBits;
	ret.info.scalar.traits = LEMNI_SCALAR_RANGE | LEMNI_SCALAR_RATIO | LEMNI_SCALAR_REAL;
	return ret;
}

static inline constexpr auto realTypeInfo(const uint32_t numBits = 0) noexcept{
	auto ret = numberTypeInfo();
	ret.info.scalar.numBits = numBits;
	ret.info.scalar.traits = LEMNI_SCALAR_RANGE | LEMNI_SCALAR_REAL;
	return ret;
}

static inline constexpr auto strTypeInfo() noexcept{
	constexpr LemniBinaryOp binaryOps[] = {
		LEMNI_BINARY_CONCAT
	};

	auto ret = scalarTypeInfo(LEMNI_SCALAR_TEXTUAL, 128);
	ret.binaryOpFlags = binaryOpFlags(binaryOps);
	return ret;
}

static inline constexpr auto strUtf8TypeInfo() noexcept{
	auto ret = strTypeInfo();
	ret.info.scalar.traits |= LEMNI_SCALAR_UTF8;
	return ret;
}

static inline constexpr auto strAsciiTypeInfo() noexcept{
	auto ret = strTypeInfo();
	ret.info.scalar.traits |= LEMNI_SCALAR_ASCII;
	return ret;
}

static inline constexpr auto callableTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_CALLABLE;
	ret.info.callable.resultTypeIdx = UINT64_MAX;
	ret.info.callable.numParams = 0;
	ret.info.callable.numClosed = 0;
	ret.info.callable.paramTypeIndices = nullptr;
	ret.info.callable.closedTypeIndices = nullptr;
	return ret;
}

//! \warning \p paramTypeIndices must stay valid for the life of the type info
static inline auto functionTypeInfo(const uint64_t resultTypeIdx, const uint64_t numParams, const uint64_t *paramTypeIndices) noexcept{
	auto ret = callableTypeInfo();
	ret.info.callable.resultTypeIdx = resultTypeIdx;
	ret.info.callable.numParams = numParams;
	ret.info.callable.paramTypeIndices = paramTypeIndices;
	return ret;
}

static inline auto closureTypeInfo(
	const uint64_t resultTypeIdx,
	const uint64_t numParams, const uint64_t *paramTypeIndices,
	const uint64_t numClosed, const uint64_t *closedTypeIndices
) noexcept
{
	auto ret = functionTypeInfo(resultTypeIdx, numParams, paramTypeIndices);
	ret.info.callable.numClosed = numClosed;
	ret.info.callable.closedTypeIndices = closedTypeIndices;
	return ret;
}

static inline constexpr auto sigmaTypeInfo(const uint64_t elemTypeIdx, const uint64_t numElems) noexcept{
	constexpr LemniBinaryOp binaryOps[] = {
		LEMNI_BINARY_CONCAT
	};

	auto ret = zeroedTypeInfo();
	ret.binaryOpFlags = binaryOpFlags(binaryOps);
	ret.typeClass = LEMNI_TYPECLASS_SIGMA;
	ret.info.sigma.numElems = numElems;
	ret.info.sigma.elemTypeIdx = elemTypeIdx;
	return ret;
}

inline static constexpr bool typeHasBinaryOp(LemniTypeInfo info, const LemniBinaryOp op) noexcept{
	return info.binaryOpFlags & op;
}

inline static constexpr bool typeHasUnaryOp(LemniTypeInfo info, const LemniUnaryOp op) noexcept{
	return info.unaryOpFlags & op;
}

enum class LemniTypeCategory: uint32_t{
	scalar,
	pseudo,
	meta,
	module,
	callable,
	compound,
	top, bottom,

	count
};

enum class LemniScalarFlags: uint32_t{
	range,
	boolean, natural, integer, ratio, real,
	textual,
	ascii, utf8,

	count
};

enum class LemniMetaFlags: uint32_t{
	list, atom,

	count
};

enum class LemniCallableFlags: uint32_t{
	closure,

	count
};

enum class LemniCompoundFlags: uint32_t{
	array, sum, product, record,

	count
};

enum class LemniModuleFlags: uint32_t{
	count
};

template<typename Enum>
inline constexpr uint32_t enumFlag(Enum val) noexcept{ return uint32_t(1) << static_cast<uint32_t>(val); }

struct LemniTypeTraitsT{
	LemniTypeCategory category;
	uint32_t flags;
};

template<typename TypeBase, typename Base, typename T, typename = void>
struct LemniTypeBaseImplT;

template<typename TypeBase, typename Base, typename T>
struct LemniTypeBaseImplT<TypeBase, Base, T, std::enable_if_t<std::is_base_of_v<TypeBase, Base>>>: Base{
	LemniTypeBaseImplT(LemniType base_, LemniType abstract_, const LemniNat32 numBits_, const LemniNat64 typeInfoIdx_, std::string str_, std::string mangled_)
		: m_base(base_), m_abstract(abstract_), m_numBits(numBits_), m_typeInfoIdx(typeInfoIdx_), m_str(std::move(str_)), m_mangled(std::move(mangled_)){}

	virtual ~LemniTypeBaseImplT() = default;

	virtual bool isSame(LemniType other) const noexcept override{
		return (this == other) || (m_mangled == lemni::toStdStrView(other->mangled()));
	}

	virtual bool isCastable(LemniType to) const noexcept override{
		return (to == this) || lemniTypeAsPseudo(to);
	}

	LemniStr str() const noexcept override{ return lemni::fromStdStrView(m_str); }
	LemniStr mangled() const noexcept override{ return lemni::fromStdStrView(m_mangled); }
	LemniType base() const noexcept override{ return m_base; }
	LemniType abstract() const noexcept override{ return m_abstract; }
	LemniNat32 numBits() const noexcept override{ return m_numBits; }
	LemniNat64 typeIdx() const noexcept override{ return m_typeInfoIdx; }

	LemniType m_base, m_abstract;
	LemniNat32 m_numBits;
	LemniNat64 m_typeInfoIdx;
	std::string m_str, m_mangled;
};

template<typename Base, typename T>
struct LemniTypeImplT: LemniTypeBaseImplT<LemniTypeT, Base, T>{
	using LemniTypeBaseImplT<LemniTypeT, Base, T>::LemniTypeBaseImplT;
};

template<typename Base, typename T>
struct LemniCTypeImplT: LemniTypeBaseImplT<LemniCTypeT, Base, T>{
	using LemniTypeBaseImplT<LemniCTypeT, Base, T>::LemniTypeBaseImplT;
};

struct LemniTopTypeImplT: LemniTypeImplT<LemniTopTypeT, LemniTopTypeImplT>{
	LemniTopTypeImplT(LemniNat64 typeInfoIdx_): LemniTypeImplT(this, this, 0, typeInfoIdx_, "Top", "@"){}
};

struct LemniBottomTypeImplT: LemniTypeImplT<LemniBottomTypeT, LemniBottomTypeImplT>{
	LemniBottomTypeImplT(LemniTopType top, const LemniNat64 typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Bottom", "!"){}
};

struct LemniModuleTypeImplT: LemniTypeImplT<LemniModuleTypeT, LemniModuleTypeImplT>{
	LemniModuleTypeImplT(LemniTopType top, const LemniNat64 typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Module", "#"){}
};

struct LemniPseudoTypeImplT: LemniTypeImplT<LemniPseudoTypeT, LemniPseudoTypeImplT>{
	LemniPseudoTypeImplT(LemniTopType top, const LemniNat64 typeInfoIdx_, const LemniNat64 pseudoIdx)
		: LemniTypeImplT(top, this, 0, typeInfoIdx_, "Pseudo " + std::to_string(pseudoIdx), "?" + std::to_string(pseudoIdx)){}
};

struct LemniExprTypeImplT: LemniTypeImplT<LemniExprTypeT, LemniExprTypeImplT>{
	LemniExprTypeImplT(LemniTopType top, const LemniNat64 typeInfoIdx_)
		: LemniTypeImplT(top, this, 0, typeInfoIdx_, "Expr", "e0"){}
};

struct LemniMetaTypeImplT: LemniTypeImplT<LemniMetaTypeT, LemniMetaTypeImplT>{
	LemniMetaTypeImplT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Type", "m0"){}
};

struct LemniCPtrTypeImplT: LemniCTypeImplT<LemniCPtrTypeT, LemniCPtrTypeImplT>{
	LemniCPtrTypeImplT(LemniTopType base_, LemniType pointed_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, sizeof(void*) * __CHAR_BIT__, typeInfoIdx_, "Ptr " + lemni::toStdStr(pointed_->str()), "ip" + lemni::toStdStr(pointed_->mangled()))
		, pointed(pointed_)
	{}

	LemniType pointed;
};

struct LemniCConstTypeImplT: LemniCTypeImplT<LemniCConstTypeT, LemniCConstTypeImplT>{
	LemniCConstTypeImplT(LemniTopType base_, LemniType qualified_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, qualified_->numBits(), typeInfoIdx_, "Const " + lemni::toStdStr(qualified_->str()), "ic" + lemni::toStdStr(qualified_->mangled()))
		, qualified(qualified_)
	{}

	LemniType qualified;
};

struct LemniCVoidTypeImplT: LemniCTypeImplT<LemniCVoidTypeT, LemniCVoidTypeImplT>{
	LemniCVoidTypeImplT(LemniTopType base_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, 0, typeInfoIdx_, "Void", "iv0"){}
};


struct LemniUnitTypeImplT: LemniTypeImplT<LemniUnitTypeT, LemniUnitTypeImplT>{
	explicit LemniUnitTypeImplT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Unit", "u0"){}
};

struct LemniBoolTypeImplT: LemniTypeImplT<LemniBoolTypeT, LemniBoolTypeImplT>{
	explicit LemniBoolTypeImplT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 1, typeInfoIdx_, "Bool", "b8"){}
};

struct LemniNumberTypeImplT: LemniTypeImplT<LemniNumberTypeT, LemniNumberTypeImplT>{
	explicit LemniNumberTypeImplT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Number", "x0"){}
};

struct LemniRealTypeImplT: LemniTypeImplT<LemniRealTypeT, LemniRealTypeImplT>{
	LemniRealTypeImplT(LemniNumberType base, LemniRealType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Real" + (numBits > 0 ? std::to_string(numBits) : ""), "r" + std::to_string(numBits)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniRatioTypeImplT: LemniTypeImplT<LemniRatioTypeT, LemniRatioTypeImplT>{
	LemniRatioTypeImplT(LemniRealType base, LemniRatioType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits, LemniIntType numType_, LemniNatType denType_)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Ratio" + (numBits > 0 ? std::to_string(numBits) : ""s), "q" + std::to_string(numBits))
		, numType(numType_), denType(denType_){}

	bool isCastable(LemniType to) const noexcept override;

	LemniIntType numerator() const noexcept override{ return numType; }
	LemniNatType denominator() const noexcept override{ return denType; }

	LemniIntType numType;
	LemniNatType denType;
};

struct LemniIntTypeImplT: LemniTypeImplT<LemniIntTypeT, LemniIntTypeImplT>{
	LemniIntTypeImplT(LemniRatioType base, LemniIntType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Int" + (numBits > 0 ? std::to_string(numBits) : ""s), "z" + std::to_string(numBits)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniNatTypeImplT: LemniTypeImplT<LemniNatTypeT, LemniNatTypeImplT>{
	LemniNatTypeImplT(LemniIntType base, LemniNatType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits_)
		: LemniTypeImplT(base, abstract, numBits_, typeInfoIdx_, "Nat" + (numBits_ > 0 ? std::to_string(numBits_) : ""s), "n" + std::to_string(numBits_)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniStringTypeImplT: LemniTypeImplT<LemniStringTypeT, LemniStringTypeImplT>{
	LemniStringTypeImplT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "String", "s0"){}
};

struct LemniStringASCIITypeImplT: LemniTypeImplT<LemniStringASCIITypeT, LemniStringASCIITypeImplT>{
	LemniStringASCIITypeImplT(LemniStringType str, const uint64_t typeInfoIdx_): LemniTypeImplT(str, this, 0, typeInfoIdx_, "StringASCII", "sa8"){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniStringUTF8TypeImplT: LemniTypeImplT<LemniStringUTF8TypeT, LemniStringUTF8TypeImplT>{
	explicit LemniStringUTF8TypeImplT(LemniStringASCIIType str, const uint64_t typeInfoIdx_): LemniTypeImplT(str, this, 0, typeInfoIdx_, "StringUTF8", "su8"){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniFunctionTypeImplT: LemniTypeImplT<LemniFunctionTypeT, LemniFunctionTypeImplT>{
	LemniFunctionTypeImplT(LemniTopType base, const uint64_t typeInfoIdx_, LemniType result_, std::vector<LemniType> params_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, lemni::toStdStr(result_->str()), "f" + std::to_string(params_.size()) + lemni::toStdStr(result_->mangled()))
		, m_result(result_), m_params(std::move(params_))
	{
		std::string mangledParams;

		for(auto it = rbegin(m_params); it != rend(m_params); ++it){
			mangledParams.insert(0, lemni::toStdStrView((*it)->mangled()));
			m_str.insert(0, " -> ");
			m_str.insert(0, lemni::toStdStrView((*it)->str()));
		}

		m_mangled.append(mangledParams);
	}

	//bool isCastable(LemniType to) const noexcept override;

	LemniType result() const noexcept override{ return m_result; }
	LemniNat64 numParams() const noexcept override{ return m_params.size(); }
	LemniType param(const LemniNat64 idx) const noexcept override{ return m_params[idx]; }

	LemniType m_result;
	std::vector<LemniType> m_params;
};

struct LemniArrayTypeImplT: LemniTypeImplT<LemniArrayTypeT, LemniArrayTypeImplT>{
	LemniArrayTypeImplT(LemniTopType base, const uint64_t typeInfoIdx_, const uint64_t numElements_, LemniType elementType_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, "[" + (numElements_ > 0 ? std::to_string(numElements_) : "") + "]", "a" + std::to_string(numElements_) + lemni::toStdStr(elementType_->mangled()))
		, m_numElements(numElements_), m_elementType(elementType_)
	{
		m_str += lemni::toStdStrView(m_elementType->str());
	}

	bool isCastable(LemniType to) const noexcept override;

	LemniType element() const noexcept override{ return m_elementType; }
	LemniNat64 numElements() const noexcept override{ return m_numElements; }

	const uint64_t m_numElements;
	LemniType m_elementType;
};

struct LemniClosureTypeImplT: LemniTypeImplT<LemniClosureTypeT, LemniClosureTypeImplT>{
	LemniClosureTypeImplT(LemniFunctionType base, const uint64_t typeInfoIdx_, std::vector<LemniType> closed_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, lemni::toStdStr(base->str()), "g" + std::to_string(closed_.size()) + lemni::toStdStr(base->mangled()))
		, m_closed(std::move(closed_))
	{
		// TODO: add closure environment to type string
	}

	//bool isCastable(LemniType to) const noexcept override;

	LemniFunctionType fn() const noexcept override{ return reinterpret_cast<LemniFunctionType>(m_base); }
	LemniNat64 numClosed() const noexcept override{ return m_closed.size(); }
	LemniType closed(const LemniNat64 idx) const noexcept override{ return m_closed[idx]; }

	std::vector<LemniType> m_closed;
};

struct LemniSumTypeImplT: LemniTypeImplT<LemniSumTypeT, LemniSumTypeImplT>{
	LemniSumTypeImplT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniType> cases_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, lemni::toStdStr(cases_[0]->str()), "u" + std::to_string(cases_.size()) + lemni::toStdStr(cases_[0]->mangled()))
		, cases(std::move(cases_))
	{
		for(std::size_t i = 1; i < cases.size(); i++){
			this->m_str += " | ";
			this->m_str += lemni::toStdStrView(cases[i]->str());
			this->m_mangled += lemni::toStdStrView(cases[i]->mangled());
		}
	}

	LemniNat64 numCases() const noexcept override{ return cases.size(); }
	LemniType case_(const LemniNat64 idx) const noexcept override{ return cases[idx]; }

	std::vector<LemniType> cases;
};

struct LemniProductTypeImplT: LemniTypeImplT<LemniProductTypeT, LemniProductTypeImplT>{
	LemniProductTypeImplT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniType> components_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, lemni::toStdStr(components_[0]->str()), "t" + std::to_string(components_.size()) + lemni::toStdStr(components_[0]->mangled()))
		, components(std::move(components_))
	{
		for(std::size_t i = 1; i < components.size(); i++){
			this->m_str += " & ";
			this->m_str += lemni::toStdStrView(components[i]->str());
			this->m_mangled += lemni::toStdStrView(components[i]->mangled());
		}
	}

	bool isCastable(LemniType to) const noexcept override;

	LemniNat64 numComponents() const noexcept override{ return components.size(); }
	LemniType component(const LemniNat64 idx) const noexcept override{ return components[idx]; }

	std::vector<LemniType> components;
};

struct LemniRecordTypeImplT: LemniTypeImplT<LemniRecordTypeT, LemniRecordTypeImplT>{
	LemniRecordTypeImplT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniRecordTypeField> fields_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, "Record", "o" + std::to_string(fields_.size()))
		, fields(std::move(fields_))
	{
		for(std::size_t i = 0; i < fields.size(); i++){
			auto &&field = fields[i];
			this->m_mangled += lemni::toStdStrView(field.type->mangled());
			this->m_mangled += std::to_string(field.name.len);
			this->m_mangled += lemni::toStdStrView(field.name);
		}
	}

	LemniNat64 numFields() const noexcept override{ return fields.size(); }
	const LemniRecordTypeField *field(const LemniNat64 idx) const noexcept override{ return &fields[idx]; }

	std::vector<LemniRecordTypeField> fields;
};

#endif // !LEMNI_LIB_TYPE_HPP
