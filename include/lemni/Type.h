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

#ifndef LEMNI_TYPE_H
#define LEMNI_TYPE_H 1

#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct LemniTypeSetT *LemniTypeSet;

typedef const struct LemniTypeT *LemniType;

typedef const struct LemniTopTypeT *LemniTopType;
typedef const struct LemniBottomTypeT *LemniBottomType;
typedef const struct LemniUnitTypeT *LemniUnitType;
typedef const struct LemniBoolTypeT *LemniBoolType;

typedef const struct LemniNumberTypeT *LemniNumberType;
typedef const struct LemniRealTypeT *LemniRealType;
typedef const struct LemniRatioTypeT *LemniRatioType;
typedef const struct LemniIntTypeT *LemniIntType;
typedef const struct LemniNatTypeT *LemniNatType;

typedef const struct LemniStringTypeT *LemniStringType;
typedef const struct LemniStringUTF8TypeT *LemniStringUTF8Type;
typedef const struct LemniStringASCIITypeT *LemniStringASCIIType;

typedef const struct LemniFunctionTypeT *LemniFunctionType;

typedef const struct LemniSumTypeT *LemniSumType;
typedef const struct LemniProductTypeT *LemniProductType;

typedef const struct LemniRecordTypeT *LemniRecordType;

typedef struct {
	LemniStr name;
	LemniType type;
} LemniRecordTypeField;

LemniType lemniTypeBase(LemniType type);
LemniType lemniTypeAbstract(LemniType type);
uint32_t lemniTypeNumBits(LemniType type);

LemniTopType lemniTypeAsTop(LemniType type);
LemniType lemniTopAsType(LemniTopType top);

LemniBottomType lemniTypeAsBottom(LemniType type);
LemniTopType lemniBottomTypeBase(LemniBottomType bottom);
LemniType lemniBottomAsType(LemniBottomType bottom);

LemniUnitType lemniTypeAsUnit(LemniType type);
LemniTopType lemniUnitTypeBase(LemniUnitType unit);
LemniType lemniUnitAsType(LemniUnitType unit);

LemniBoolType lemniTypeAsBool(LemniType type);
LemniTopType lemniBoolTypeBase(LemniBoolType bool_);
LemniType lemniBoolAsType(LemniBoolType bool_);

LemniNumberType lemniTypeAsNumber(LemniType type);
LemniTopType LemniNumberTypeBase(LemniNumberType num);

LemniRealType lemniTypeAsReal(LemniType type);
LemniNumberType lemniRealTypeBase(LemniRealType real);
LemniRealType lemniRealTypeAbstract(LemniRealType real);

LemniRatioType lemniTypeAsRatio(LemniType type);
LemniRealType lemniRatioTypeBase(LemniRatioType ratio);
LemniRatioType lemniRatioTypeAbstract(LemniRatioType ratio);

LemniIntType lemniTypeAsInt(LemniType type);
LemniRatioType lemniIntTypeBase(LemniIntType int_);
LemniIntType lemniIntTypeAbstract(LemniIntType int_);

LemniNatType lemniTypeAsNat(LemniType type);
LemniIntType lemniNatTypeBase(LemniNatType nat);
LemniNatType lemniNatTypeAbstract(LemniNatType nat);

LemniStringType lemniTypeAsString(LemniType type);
LemniTopType lemniStringTypeBase(LemniStringType str);

LemniStringASCIIType lemniTypeAsStringASCII(LemniType type);
LemniStringType lemniStringASCIITypeBase(LemniStringASCIIType strA);

LemniStringUTF8Type lemniTypeAsStringUTF8(LemniType type);
LemniStringASCIIType lemniStringUTF8TypeBase(LemniStringUTF8Type strU);

LemniFunctionType lemniTypeAsFunction(LemniType type);
LemniTopType lemniFunctionTypeBase(LemniFunctionType fn);
LemniType lemniFunctionTypeResult(LemniFunctionType fn);
uint32_t lemniFunctionTypeNumParams(LemniFunctionType fn);
LemniType lemniFunctionTypeParam(LemniFunctionType fn, const uint32_t idx);

LemniSumType lemniTypeAsSum(LemniType type);
LemniTopType lemniSumTypeBase(LemniSumType sum);
uint32_t lemniSumTypeNumCases(LemniSumType sum);
LemniType lemniSumTypeCase(LemniSumType sum, const uint32_t idx);

LemniProductType lemniTypeAsProduct(LemniType type);
LemniTopType lemniProductTypeBase(LemniProductType product);
uint32_t lemniProductTypeNumComponents(LemniProductType product);
LemniType lemniProductTypeComponent(LemniProductType product, const uint32_t idx);

LemniRecordType lemniTypeAsRecord(LemniType type);
LemniTopType lemniRecordTypeBase(LemniRecordType record);
uint32_t lemniRecordTypeNumFields(LemniRecordType record);
const LemniRecordTypeField *lemniRecordTypeField(LemniRecordType record, const uint32_t idx);

LemniTypeSet lemniCreateTypeSet();

void lemniDestroyTypeSet(LemniTypeSet types);

LemniTopType lemniTypeSetGetTop(LemniTypeSet types);
LemniBottomType lemniTypeSetGetBottom(LemniTypeSet types);
LemniUnitType lemniTypeSetGetUnit(LemniTypeSet types);
LemniBoolType lemniTypeSetGetBool(LemniTypeSet types);

LemniNumberType lemniTypeSetGetNumber(LemniTypeSet types);
LemniRealType lemniTypeSetGetReal(LemniTypeSet types, uint32_t numBits);
LemniRatioType lemniTypeSetGetRatio(LemniTypeSet types, uint32_t numBits);
LemniIntType lemniTypeSetGetInt(LemniTypeSet types, uint32_t numBits);
LemniNatType lemniTypeSetGetNat(LemniTypeSet types, uint32_t numBits);

LemniStringType lemniTypeSetGetString(LemniTypeSet types);
LemniStringASCIIType lemniTypeSetGetStringASCII(LemniTypeSet types);
LemniStringUTF8Type lemniTypeSetGetStringUTF8(LemniTypeSet types);

LemniSumType lemniTypeSetGetSum(LemniTypeSet types, LemniType *const cases, uint32_t numCases);

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, uint32_t numComponents);

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, uint32_t numFields);

#ifdef __cplusplus
}
#endif

#endif // !LEMNI_TYPE_H
