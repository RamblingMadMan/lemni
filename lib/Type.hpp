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

#include <vector>

#define LEMNI_NO_CPP
#include "lemni/Type.h"

struct LemniTypeT{
	LemniTypeT(LemniType base_, LemniType abstract_, uint32_t numBits_, uint32_t categoryMask_)
		: base(base_), abstract(abstract_), numBits(numBits_), categoryMask(categoryMask_){}

	virtual ~LemniTypeT() = default;

	LemniType base, abstract;
	uint32_t numBits;
	uint32_t categoryMask;
};

struct LemniTopTypeT: LemniTypeT{
	LemniTopTypeT(): LemniTypeT(this, this, 0, 0xFFFFFFFF){}
};

struct LemniBottomTypeT: LemniTypeT{
	explicit LemniBottomTypeT(LemniTopType top): LemniTypeT(top, this, 0, 0){}
};

struct LemniPseudoTypeT: LemniTypeT{
	explicit LemniPseudoTypeT(LemniTopType top, uint32_t categoryMask): LemniTypeT(top, this, 0, categoryMask){}
};

struct LemniMetaTypeT: LemniTypeT{
	explicit LemniMetaTypeT(LemniTopType top): LemniTypeT(top, this, 0, 0){}
};

struct LemniUnitTypeT: LemniTypeT{
	explicit LemniUnitTypeT(LemniTopType top): LemniTypeT(top, this, 0, LEMNI_TYPE_UNIT){}
};

struct LemniBoolTypeT: LemniTypeT{
	explicit LemniBoolTypeT(LemniTopType top): LemniTypeT(top, this, 1, LEMNI_TYPE_NATURAL){}
};

struct LemniNumberTypeT: LemniTypeT{
	explicit LemniNumberTypeT(LemniTopType top): LemniTypeT(top, this, 0, LEMNI_TYPE_REAL | LEMNI_TYPE_RATIONAL | LEMNI_TYPE_INTEGRAL | LEMNI_TYPE_NATURAL){}
};

struct LemniRealTypeT: LemniTypeT{
	LemniRealTypeT(LemniNumberType base, LemniRealType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits, LEMNI_TYPE_REAL){}
};

struct LemniRatioTypeT: LemniTypeT{
	LemniRatioTypeT(LemniRealType base, LemniRatioType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits, LEMNI_TYPE_REAL | LEMNI_TYPE_RATIONAL){}
};

struct LemniIntTypeT: LemniTypeT{
	LemniIntTypeT(LemniRatioType base, LemniIntType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits, LEMNI_TYPE_REAL | LEMNI_TYPE_RATIONAL | LEMNI_TYPE_INTEGRAL){}
};

struct LemniNatTypeT: LemniTypeT{
	LemniNatTypeT(LemniIntType base, LemniNatType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits, LEMNI_TYPE_REAL | LEMNI_TYPE_RATIONAL | LEMNI_TYPE_INTEGRAL | LEMNI_TYPE_NATURAL){}
};

struct LemniStringTypeT: LemniTypeT{
	explicit LemniStringTypeT(LemniTopType top): LemniTypeT(top, this, 0, LEMNI_TYPE_TEXTUAL){}
};

struct LemniStringASCIITypeT: LemniTypeT{
	explicit LemniStringASCIITypeT(LemniStringType str): LemniTypeT(str, this, 0, LEMNI_TYPE_TEXTUAL){}
};

struct LemniStringUTF8TypeT: LemniTypeT{
	explicit LemniStringUTF8TypeT(LemniStringASCIIType str): LemniTypeT(str, this, 0, LEMNI_TYPE_TEXTUAL){}
};

struct LemniFunctionTypeT: LemniTypeT{
	LemniFunctionTypeT(LemniTopType base, LemniType result_, std::vector<LemniType> params_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_CALLABLE), result(result_), params(std::move(params_)){}

	LemniType result;
	std::vector<LemniType> params;
};

struct LemniArrayTypeT: LemniTypeT{
	LemniArrayTypeT(LemniTopType base, const uint64_t numElements_, LemniType elementType_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_ARRAY), numElements(numElements_), elementType(elementType_){}

	const uint64_t numElements;
	LemniType elementType;
};

struct LemniClosureTypeT: LemniTypeT{
	LemniClosureTypeT(LemniFunctionType base, std::vector<LemniType> closed_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_CALLABLE | LEMNI_TYPE_PRODUCT), closed(std::move(closed_)){}

	std::vector<LemniType> closed;
};

struct LemniSumTypeT: LemniTypeT{
	LemniSumTypeT(LemniTopType base, std::vector<LemniType> cases_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_SUM), cases(std::move(cases_)){}

	std::vector<LemniType> cases;
};

struct LemniProductTypeT: LemniTypeT{
	LemniProductTypeT(LemniTopType base, std::vector<LemniType> components_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_PRODUCT), components(std::move(components_)){}

	std::vector<LemniType> components;
};

struct LemniRecordTypeT: LemniTypeT{
	LemniRecordTypeT(LemniTopType base, std::vector<LemniRecordTypeField> fields_)
		: LemniTypeT(base, this, 0, LEMNI_TYPE_RECORD), fields(std::move(fields_)){}

	std::vector<LemniRecordTypeField> fields;
};

#endif // !LEMNI_LIB_TYPE_HPP
