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

static inline consteval auto zeroedTypeInfo() noexcept{
	LemniTypeInfo info;
	info.binaryOpFlags = 0;
	info.unaryOpFlags = 0;
	info.typeClass = LEMNI_TYPECLASS_EMPTY;

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

static inline consteval auto topTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_TOP;
	return ret;
}

static inline consteval auto bottomTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_BOTTOM;
	return ret;
}

static inline consteval auto metaTypeInfo() noexcept{
	auto ret = zeroedTypeInfo();
	ret.typeClass = LEMNI_TYPECLASS_META;
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

static inline consteval auto unitTypeInfo() noexcept{
	return scalarTypeInfo(LEMNI_SCALAR_UNIT);
}

static inline consteval auto boolTypeInfo() noexcept{
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

static inline consteval auto numberTypeInfo() noexcept{
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

static inline consteval auto strTypeInfo() noexcept{
	constexpr LemniBinaryOp binaryOps[] = {
		LEMNI_BINARY_CONCAT
	};

	auto ret = scalarTypeInfo(LEMNI_SCALAR_TEXTUAL, 128);
	ret.binaryOpFlags = binaryOpFlags(binaryOps);
	return ret;
}

static inline consteval auto strUtf8TypeInfo() noexcept{
	auto ret = strTypeInfo();
	ret.info.scalar.traits |= LEMNI_SCALAR_UTF8;
	return ret;
}

static inline consteval auto strAsciiTypeInfo() noexcept{
	auto ret = strTypeInfo();
	ret.info.scalar.traits |= LEMNI_SCALAR_ASCII;
	return ret;
}

static inline consteval auto callableTypeInfo() noexcept{
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

template<typename T>
struct LemniTypeImplT: public LemniTypeT{
	LemniTypeImplT(LemniType base_, LemniType abstract_, const uint32_t numBits_, const uint64_t typeInfoIdx_, std::string str_, std::string mangled_)
		: m_base(base_), m_abstract(abstract_), m_numBits(numBits_), m_typeInfoIdx(typeInfoIdx_), m_str(std::move(str_)), m_mangled(std::move(mangled_)){}

	virtual ~LemniTypeImplT() = default;

	virtual bool isSame(LemniType other) const noexcept override{
		return (this == other) || (m_mangled == other->mangled());
	}

	virtual bool isCastable(LemniType to) const noexcept override{
		return (to == this) || lemniTypeAsPseudo(to);
	}

	std::string_view str() const noexcept override{ return m_str; }
	std::string_view mangled() const noexcept override{ return m_mangled; }
	LemniType base() const noexcept override{ return m_base; }
	LemniType abstract() const noexcept override{ return m_abstract; }
	uint32_t numBits() const noexcept override{ return m_numBits; }
	uint64_t typeIdx() const noexcept override{ return m_typeInfoIdx; }

	LemniType m_base, m_abstract;
	uint32_t m_numBits;
	uint64_t m_typeInfoIdx;
	std::string m_str, m_mangled;
};

template<typename T>
struct LemniCTypeImplT: public LemniCTypeT{
	LemniCTypeImplT(LemniType base_, LemniType abstract_, const uint32_t numBits_, const uint64_t typeInfoIdx_, std::string str_, std::string mangled_)
		: m_base(base_), m_abstract(abstract_), m_numBits(numBits_), m_typeInfoIdx(typeInfoIdx_), m_str(std::move(str_)), m_mangled(std::move(mangled_)){}

	virtual ~LemniCTypeImplT() = default;

	virtual bool isSame(LemniType other) const noexcept override{
		return (this == other) || (m_mangled == other->mangled());
	}

	virtual bool isCastable(LemniType to) const noexcept override{
		return (to == this) || lemniTypeAsPseudo(to);
	}

	std::string_view str() const noexcept override{ return m_str; }
	std::string_view mangled() const noexcept override{ return m_mangled; }
	LemniType base() const noexcept override{ return m_base; }
	LemniType abstract() const noexcept override{ return m_abstract; }
	uint32_t numBits() const noexcept override{ return m_numBits; }
	uint64_t typeIdx() const noexcept override{ return m_typeInfoIdx; }

	LemniType m_base, m_abstract;
	uint32_t m_numBits;
	uint64_t m_typeInfoIdx;
	std::string m_str, m_mangled;
};


struct LemniTopTypeT: LemniTypeImplT<LemniTopTypeT>{
	LemniTopTypeT(uint64_t typeInfoIdx_): LemniTypeImplT(this, this, 0, typeInfoIdx_, "Top", "@"){}
};

struct LemniBottomTypeT: LemniTypeImplT<LemniBottomTypeT>{
	LemniBottomTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Bottom", "!"){}
};

struct LemniModuleTypeT: LemniTypeImplT<LemniModuleTypeT>{
	LemniModuleTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Module", "#"){}
};

struct LemniPseudoTypeT: LemniTypeImplT<LemniPseudoTypeT>{
	LemniPseudoTypeT(LemniTopType top, const uint64_t typeInfoIdx_, const uint64_t pseudoIdx)
		: LemniTypeImplT(top, this, 0, typeInfoIdx_, "Pseudo " + std::to_string(pseudoIdx), "?" + std::to_string(pseudoIdx)){}
};

struct LemniMetaTypeT: LemniTypeImplT<LemniMetaTypeT>{
	LemniMetaTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Type", "m0"){}
};

struct LemniCPtrTypeT: LemniCTypeImplT<LemniCPtrTypeT>{
	LemniCPtrTypeT(LemniTopType base_, LemniType pointed_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, sizeof(void*) * __CHAR_BIT__, typeInfoIdx_, "Ptr " + std::string(pointed_->str()), "ip" + std::string(pointed_->mangled()))
		, pointed(pointed_)
	{}

	LemniType pointed;
};

struct LemniCConstTypeT: LemniCTypeImplT<LemniCConstTypeT>{
	LemniCConstTypeT(LemniTopType base_, LemniType qualified_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, qualified_->numBits(), typeInfoIdx_, "Const " + std::string(qualified_->str()), "ic" + std::string(qualified_->mangled()))
		, qualified(qualified_)
	{}

	LemniType qualified;
};

struct LemniCVoidTypeT: LemniCTypeImplT<LemniCVoidTypeT>{
	LemniCVoidTypeT(LemniTopType base_, const uint64_t typeInfoIdx_)
		: LemniCTypeImplT(base_, this, 0, typeInfoIdx_, "Void", "iv0"){}
};


struct LemniUnitTypeT: LemniTypeImplT<LemniUnitTypeT>{
	explicit LemniUnitTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Unit", "u0"){}
};

struct LemniBoolTypeT: LemniTypeImplT<LemniBoolTypeT>{
	explicit LemniBoolTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 1, typeInfoIdx_, "Bool", "b8"){}
};

struct LemniNumberTypeT: LemniTypeImplT<LemniNumberTypeT>{
	explicit LemniNumberTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "Number", "x0"){}
};

struct LemniRealTypeT: LemniTypeImplT<LemniRealTypeT>{
	LemniRealTypeT(LemniNumberType base, LemniRealType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Real" + (numBits > 0 ? std::to_string(numBits) : ""), "r" + std::to_string(numBits)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniRatioTypeT: LemniTypeImplT<LemniRatioTypeT>{
	LemniRatioTypeT(LemniRealType base, LemniRatioType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Ratio" + (numBits > 0 ? std::to_string(numBits) : ""s), "q" + std::to_string(numBits)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniIntTypeT: LemniTypeImplT<LemniIntTypeT>{
	LemniIntTypeT(LemniRatioType base, LemniIntType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits)
		: LemniTypeImplT(base, abstract, numBits, typeInfoIdx_, "Int" + (numBits > 0 ? std::to_string(numBits) : ""s), "z" + std::to_string(numBits)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniNatTypeT: LemniTypeImplT<LemniNatTypeT>{
	LemniNatTypeT(LemniIntType base, LemniNatType abstract, const uint64_t typeInfoIdx_, const uint32_t numBits_)
		: LemniTypeImplT(base, abstract, numBits_, typeInfoIdx_, "Nat" + (numBits_ > 0 ? std::to_string(numBits_) : ""s), "n" + std::to_string(numBits_)){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniStringTypeT: LemniTypeImplT<LemniStringTypeT>{
	LemniStringTypeT(LemniTopType top, const uint64_t typeInfoIdx_): LemniTypeImplT(top, this, 0, typeInfoIdx_, "String", "s0"){}
};

struct LemniStringASCIITypeT: LemniTypeImplT<LemniStringASCIITypeT>{
	LemniStringASCIITypeT(LemniStringType str, const uint64_t typeInfoIdx_): LemniTypeImplT(str, this, 0, typeInfoIdx_, "StringASCII", "sa8"){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniStringUTF8TypeT: LemniTypeImplT<LemniStringUTF8TypeT>{
	explicit LemniStringUTF8TypeT(LemniStringASCIIType str, const uint64_t typeInfoIdx_): LemniTypeImplT(str, this, 0, typeInfoIdx_, "StringUTF8", "su8"){}

	bool isCastable(LemniType to) const noexcept override;
};

struct LemniFunctionTypeT: LemniTypeImplT<LemniFunctionTypeT>{
	LemniFunctionTypeT(LemniTopType base, const uint64_t typeInfoIdx_, LemniType result_, std::vector<LemniType> params_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, std::string(result_->str()), "f" + std::to_string(params_.size()) + std::string(result_->mangled()))
		, result(result_), params(std::move(params_))
	{
		for(auto it = rbegin(params); it != rend(params); ++it){
			m_str.insert(0, std::string((*it)->str()) + " -> ");
		}

		for(auto param : params){
			m_mangled.append(param->mangled());
		}
	}

	//bool isCastable(LemniType to) const noexcept override;

	LemniType result;
	std::vector<LemniType> params;
};

struct LemniArrayTypeT: LemniTypeImplT<LemniArrayTypeT>{
	LemniArrayTypeT(LemniTopType base, const uint64_t typeInfoIdx_, const uint64_t numElements_, LemniType elementType_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, "[" + (numElements_ > 0 ? std::to_string(numElements_) : "") + "]", "a" + std::to_string(numElements_) + std::string(elementType_->mangled()))
		, numElements(numElements_), elementType(elementType_)
	{
		m_str += elementType->str();
	}

	bool isCastable(LemniType to) const noexcept override;

	const uint64_t numElements;
	LemniType elementType;
};

struct LemniClosureTypeT: LemniTypeImplT<LemniClosureTypeT>{
	LemniClosureTypeT(LemniFunctionType base, const uint64_t typeInfoIdx_, std::vector<LemniType> closed_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, base->m_str, "g" + std::to_string(closed_.size()) + base->m_mangled)
		, closed(std::move(closed_))
	{
		// TODO: add closure environment to type string
	}

	//bool isCastable(LemniType to) const noexcept override;

	std::vector<LemniType> closed;
};

struct LemniSumTypeT: LemniTypeImplT<LemniSumTypeT>{
	LemniSumTypeT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniType> cases_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, std::string(cases_[0]->str()), "u" + std::to_string(cases_.size()) + std::string(cases_[0]->mangled()))
		, cases(std::move(cases_))
	{
		for(std::size_t i = 1; i < cases.size(); i++){
			this->m_str += " | " + std::string(cases[i]->str());
			this->m_mangled += cases[i]->mangled();
		}
	}

	std::vector<LemniType> cases;
};

struct LemniProductTypeT: LemniTypeImplT<LemniProductTypeT>{
	LemniProductTypeT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniType> components_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, std::string(components_[0]->str()), "t" + std::to_string(components_.size()) + std::string(components_[0]->mangled()))
		, components(std::move(components_))
	{
		for(std::size_t i = 1; i < components.size(); i++){
			this->m_str += " & " + std::string(components[i]->str());
			this->m_mangled += components[i]->mangled();
		}
	}

	bool isCastable(LemniType to) const noexcept override;

	std::vector<LemniType> components;
};

struct LemniRecordTypeT: LemniTypeImplT<LemniRecordTypeT>{
	LemniRecordTypeT(LemniTopType base, const uint64_t typeInfoIdx_, std::vector<LemniRecordTypeField> fields_)
		: LemniTypeImplT(base, this, 0, typeInfoIdx_, "Record", "o" + std::to_string(fields_.size()))
		, fields(std::move(fields_))
	{
		for(std::size_t i = 0; i < fields.size(); i++){
			auto &&field = fields[i];
			this->m_mangled += field.type->mangled();
			this->m_mangled += std::to_string(field.name.len);
			this->m_mangled += lemni::toStdStrView(field.name);
		}
	}

	std::vector<LemniRecordTypeField> fields;
};

#endif // !LEMNI_LIB_TYPE_HPP
