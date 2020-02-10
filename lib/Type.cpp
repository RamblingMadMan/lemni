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
#include <new>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>

#include "lemni/Str.h"

#define LEMNI_NO_CPP
#include "lemni/Type.h"

struct LemniTypeT{
	LemniTypeT(LemniType base_, LemniType abstract_, uint32_t numBits_)
		: base(base_), abstract(abstract_), numBits(numBits_){}

	virtual ~LemniTypeT() = default;

	LemniType base, abstract;
	uint32_t numBits;
};

struct LemniTopTypeT: LemniTypeT{
	LemniTopTypeT(): LemniTypeT(this, this, 0){}
};

struct LemniBottomTypeT: LemniTypeT{
	explicit LemniBottomTypeT(LemniTopType top): LemniTypeT(top, this, 0){}
};

struct LemniUnitTypeT: LemniTypeT{
	explicit LemniUnitTypeT(LemniTopType top): LemniTypeT(top, this, 0){}
};

struct LemniBoolTypeT: LemniTypeT{
	explicit LemniBoolTypeT(LemniTopType top): LemniTypeT(top, this, 1){}
};

struct LemniNumberTypeT: LemniTypeT{
	explicit LemniNumberTypeT(LemniTopType top): LemniTypeT(top, this, 0){}
};

struct LemniRealTypeT: LemniTypeT{
	LemniRealTypeT(LemniNumberType base, LemniRealType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits){}
};

struct LemniRatioTypeT: LemniTypeT{
	LemniRatioTypeT(LemniRealType base, LemniRatioType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits){}
};

struct LemniIntTypeT: LemniTypeT{
	LemniIntTypeT(LemniRatioType base, LemniIntType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits){}
};

struct LemniNatTypeT: LemniTypeT{
	LemniNatTypeT(LemniIntType base, LemniNatType abstract, uint32_t numBits)
		: LemniTypeT(base, abstract, numBits){}
};

struct LemniStringTypeT: LemniTypeT{
	explicit LemniStringTypeT(LemniTopType top): LemniTypeT(top, this, 0){}
};

struct LemniStringASCIITypeT: LemniTypeT{
	explicit LemniStringASCIITypeT(LemniStringType str): LemniTypeT(str, this, 0){}
};

struct LemniStringUTF8TypeT: LemniTypeT{
	explicit LemniStringUTF8TypeT(LemniStringASCIIType str): LemniTypeT(str, this, 0){}
};

struct LemniFunctionTypeT: LemniTypeT{
	LemniFunctionTypeT(LemniTopType base, LemniType result_, std::vector<LemniType> params_)
		: LemniTypeT(base, this, 0), result(result_), params(std::move(params_)){}

	LemniType result;
	std::vector<LemniType> params;
};

struct LemniSumTypeT: LemniTypeT{
	LemniSumTypeT(LemniTopType base, std::vector<LemniType> cases_)
		: LemniTypeT(base, this, 0), cases(std::move(cases_)){}

	std::vector<LemniType> cases;
};

struct LemniProductTypeT: LemniTypeT{
	LemniProductTypeT(LemniTopType base, std::vector<LemniType> components_)
		: LemniTypeT(base, this, 0), components(std::move(components_)){}

	std::vector<LemniType> components;
};

struct LemniRecordTypeT: LemniTypeT{
	LemniRecordTypeT(LemniTopType base, std::vector<LemniRecordTypeField> fields_)
		: LemniTypeT(base, this, 0), fields(std::move(fields_)){}

	std::vector<LemniRecordTypeField> fields;
};

LemniType lemniTypeBase(LemniType type){ return type->base; }
LemniType lemniTypeAbstract(LemniType type){ return type->abstract; }
uint32_t lemniTypeNumBits(LemniType type){ return type->numBits; }

LemniTopType lemniTypeAsTop(LemniType type){ return dynamic_cast<LemniTopType>(type); }
LemniType lemniTopAsType(LemniTopType top){ return top; }

LemniBottomType lemniTypeAsBottom(LemniType type){ return dynamic_cast<LemniBottomType>(type); }
LemniTopType lemniBottomTypeBase(LemniBottomType bottom){ return reinterpret_cast<LemniTopType>(bottom->base); }
LemniType lemniBottomAsType(LemniBottomType bottom){ return bottom; }

LemniUnitType lemniTypeAsUnit(LemniType type){ return dynamic_cast<LemniUnitType>(type); }
LemniTopType lemniUnitTypeBase(LemniUnitType unit){ return reinterpret_cast<LemniTopType>(unit->base); }
LemniType lemniUnitAsType(LemniUnitType unit){ return unit; }

LemniBoolType lemniTypeAsBool(LemniType type){ return dynamic_cast<LemniBoolType>(type); }
LemniTopType lemniBoolTypeBase(LemniBoolType bool_){ return reinterpret_cast<LemniTopType>(bool_->base); }
LemniType lemniBoolAsType(LemniBoolType bool_){ return bool_; }

LemniNumberType lemniTypeAsNumber(LemniType type){ return dynamic_cast<LemniNumberType>(type); }
LemniTopType lemniNumberTypeBase(LemniNumberType num){ return reinterpret_cast<LemniTopType>(num->base); }
LemniType lemniNumberAsType(LemniNumberType num){ return num; }

LemniRealType lemniTypeAsReal(LemniType type){ return dynamic_cast<LemniRealType>(type); }
LemniNumberType lemniRealTypeBase(LemniRealType real){ return reinterpret_cast<LemniNumberType>(real->base); }
LemniRealType lemniRealTypeAbstract(LemniRealType real){ return reinterpret_cast<LemniRealType>(real->abstract); }
LemniType lemniRealAsType(LemniRealType real){ return real; }

LemniRatioType lemniTypeAsRatio(LemniType type){ return dynamic_cast<LemniRatioType>(type); }
LemniRealType lemniRatioTypeBase(LemniRatioType ratio){ return reinterpret_cast<LemniRealType>(ratio->base); }
LemniRatioType lemniRatioTypeAbstract(LemniRatioType ratio){ return reinterpret_cast<LemniRatioType>(ratio->abstract); }
LemniType lemniRatioAsType(LemniRatioType ratio){ return ratio; }

LemniIntType lemniTypeAsInt(LemniType type){ return dynamic_cast<LemniIntType>(type); }
LemniRatioType lemniIntTypeBase(LemniIntType int_){ return reinterpret_cast<LemniRatioType>(int_->base); }
LemniIntType lemniIntTypeAbstract(LemniIntType int_){ return reinterpret_cast<LemniIntType>(int_->abstract); }
LemniType lemniIntAsType(LemniIntType int_){ return int_; }

LemniNatType lemniTypeAsNat(LemniType type){ return dynamic_cast<LemniNatType>(type); }
LemniIntType lemniNatTypeBase(LemniNatType nat){ return reinterpret_cast<LemniIntType>(nat->base); }
LemniNatType lemniNatTypeAbstract(LemniNatType nat){ return reinterpret_cast<LemniNatType>(nat->abstract); }
LemniType lemniNatAsType(LemniNatType nat){ return nat; }

LemniStringType lemniTypeAsString(LemniType type){ return dynamic_cast<LemniStringType>(type); }
LemniTopType lemniStringTypeBase(LemniStringType str){ return reinterpret_cast<LemniTopType>(str->base); }
LemniType lemniStringAsType(LemniStringType str){ return str; }

LemniStringASCIIType lemniTypeAsStringASCII(LemniType type){ return dynamic_cast<LemniStringASCIIType>(type); }
LemniStringType lemniStringASCIITypeBase(LemniStringASCIIType strA){ return reinterpret_cast<LemniStringType>(strA->base); }

LemniStringUTF8Type lemniTypeAsStringUTF8(LemniType type){ return dynamic_cast<LemniStringUTF8Type>(type); }
LemniStringASCIIType lemniStringUTF8TypeBase(LemniStringUTF8Type strU){ return dynamic_cast<LemniStringASCIIType>(strU->base); }

LemniFunctionType lemniTypeAsFunction(LemniType type){ return dynamic_cast<LemniFunctionType>(type); }
LemniTopType lemniFunctionTypeBase(LemniFunctionType fn){ return reinterpret_cast<LemniTopType>(fn->base); }
LemniType lemniFunctionTypeResult(LemniFunctionType fn){ return fn->result; }
uint32_t lemniFunctionTypeNumParams(LemniFunctionType fn){ return static_cast<uint32_t>(fn->params.size()); }
LemniType lemniFunctionTypeParam(LemniFunctionType fn, const uint32_t idx){ return fn->params[idx]; }
LemniType lemniFunctionAsType(LemniFunctionType fn){ return fn; }

LemniSumType lemniTypeAsSum(LemniType type){ return dynamic_cast<LemniSumType>(type); }
LemniTopType lemniSumTypeBase(LemniSumType sum){ return reinterpret_cast<LemniTopType>(sum->base); }
uint32_t lemniSumTypeNumCases(LemniSumType sum){ return static_cast<uint32_t>(sum->cases.size()); }
LemniType lemniSumTypeCase(LemniSumType sum, const uint32_t idx){ return sum->cases[idx]; }
LemniType lemniSumAsType(LemniSumType sum){ return sum; }

LemniProductType lemniTypeAsProduct(LemniType type){ return dynamic_cast<LemniProductType>(type); }
LemniTopType lemniProductTypeBase(LemniProductType product){ return reinterpret_cast<LemniTopType>(product->base); }
uint32_t lemniProductTypeNumComponents(LemniProductType product){ return static_cast<uint32_t>(product->components.size()); }
LemniType lemniProductTypeComponent(LemniProductType product, const uint32_t idx){ return product->components[idx]; }
LemniType lemniProductAsType(LemniProductType product){ return product; }

LemniRecordType lemniTypeAsRecord(LemniType type){ return dynamic_cast<LemniRecordType>(type); }
LemniTopType lemniRecordTypeBase(LemniRecordType record){ return reinterpret_cast<LemniTopType>(record->base); }
uint32_t lemniRecordTypeNumFields(LemniRecordType record){ return static_cast<uint32_t>(record->fields.size()); }
const LemniRecordTypeField *lemniRecordTypeField(LemniRecordType record, const uint32_t idx){ return &record->fields[idx]; }
LemniType lemniRecordAsType(LemniRecordType record){ return record; }

inline bool operator<(const LemniRecordTypeField &lhs, const LemniRecordTypeField &rhs) noexcept{
	if(lhs.type == rhs.type) return lhs.name < rhs.name;
	else return lhs.type < rhs.type;
}

struct LemniTypeSetT{
	LemniTypeSetT()
		: top()
		, bottom(&top)
		, unit(&top)
		, bool_(&top)
		, number(&top)
		, real(&number, &real, 0)
		, ratio(&real, &ratio, 0)
		, int_(&ratio, &int_, 0)
		, nat(&int_, &nat, 0)
		, str(&top)
		, strA(&str)
		, strU(&strA)
	{}

	LemniTopTypeT top;
	LemniBottomTypeT bottom;
	LemniUnitTypeT unit;
	LemniBoolTypeT bool_;
	LemniNumberTypeT number;
	LemniRealTypeT real;
	LemniRatioTypeT ratio;
	LemniIntTypeT int_;
	LemniNatTypeT nat;
	LemniStringTypeT str;
	LemniStringASCIITypeT strA;
	LemniStringUTF8TypeT strU;

	std::map<uint32_t, std::unique_ptr<LemniRealTypeT>> realTys;
	std::map<uint32_t, std::unique_ptr<LemniRatioTypeT>> ratioTys;
	std::map<uint32_t, std::unique_ptr<LemniIntTypeT>> intTys;
	std::map<uint32_t, std::unique_ptr<LemniNatTypeT>> natTys;

	std::map<std::vector<LemniType>, std::unique_ptr<LemniSumTypeT>> sumTys;
	std::map<std::vector<LemniType>, std::unique_ptr<LemniProductTypeT>> productTys;
	std::map<std::vector<LemniRecordTypeField>, std::unique_ptr<LemniRecordTypeT>> recordTys;

	std::map<LemniType, std::map<std::vector<LemniType>, std::unique_ptr<LemniFunctionTypeT>>> fnTys;
};

LemniTypeSet lemniCreateTypeSet(void){
	auto mem = std::malloc(sizeof(LemniTypeSetT));
	auto p = new(mem) LemniTypeSetT;
	return p;
}

void lemniDestroyTypeSet(LemniTypeSet types){
	std::destroy_at(types);
	std::free(types);
}

LemniTopType lemniTypeSetGetTop(LemniTypeSet types){ return &types->top; }
LemniBottomType lemniTypeSetGetBottom(LemniTypeSet types){ return &types->bottom; }
LemniUnitType lemniTypeSetGetUnit(LemniTypeSet types){ return &types->unit; }
LemniBoolType lemniTypeSetGetBool(LemniTypeSet types){ return &types->bool_; }

LemniNumberType lemniTypeSetGetNumber(LemniTypeSet types){ return &types->number; }

LemniRealType lemniTypeSetGetReal(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->real;

	auto res = types->realTys.find(numBits);
	if(res != end(types->realTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniRealTypeT>(&types->number, &types->real, numBits);

	auto emplaceRes = types->realTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniRatioType lemniTypeSetGetRatio(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->ratio;

	auto res = types->ratioTys.find(numBits);
	if(res != end(types->ratioTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniRatioTypeT>(lemniTypeSetGetReal(types, numBits / 2), &types->ratio, numBits);

	auto emplaceRes = types->ratioTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniIntType lemniTypeSetGetInt(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->int_;

	auto res = types->intTys.find(numBits);
	if(res != end(types->intTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniIntTypeT>(lemniTypeSetGetRatio(types, numBits * 2), &types->int_, numBits);

	auto emplaceRes = types->intTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniNatType lemniTypeSetGetNat(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->nat;

	auto res = types->natTys.find(numBits);
	if(res != end(types->natTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniNatTypeT>(lemniTypeSetGetInt(types, numBits + 1), &types->nat, numBits);

	auto emplaceRes = types->natTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}


LemniStringType lemniTypeSetGetString(LemniTypeSet types){ return &types->str; }
LemniStringASCIIType lemniTypeSetGetStringASCII(LemniTypeSet types){ return &types->strA; }
LemniStringUTF8Type lemniTypeSetGetStringUTF8(LemniTypeSet types){ return &types->strU; }

LemniSumType lemniTypeSetGetSum(LemniTypeSet types, LemniType *const cases, const uint32_t numCases){
	std::vector<LemniType> caseTys(cases, cases + numCases);
	std::sort(begin(caseTys), end(caseTys));
	caseTys.erase(std::unique(begin(caseTys), end(caseTys)), end(caseTys));

	auto res = types->sumTys.find(caseTys);
	if(res != end(types->sumTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniSumTypeT>(&types->top, caseTys);

	auto emplaceRes = types->sumTys.try_emplace(caseTys, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, const uint32_t numComponents){
	std::vector<LemniType> componentTys(components, components + numComponents);

	auto res = types->productTys.find(componentTys);
	if(res != end(types->productTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniProductTypeT>(&types->top, componentTys);

	auto emplaceRes = types->productTys.try_emplace(componentTys, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, const uint32_t numFields){
	std::vector<LemniRecordTypeField> fieldVals(fields, fields + numFields);

	auto res = types->recordTys.find(fieldVals);
	if(res != end(types->recordTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniRecordTypeT>(&types->top, fieldVals);

	auto emplaceRes = types->recordTys.try_emplace(fieldVals, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniFunctionType lemniTypeSetGetFunction(LemniTypeSet types, LemniType result, LemniType *const params, const uint32_t numParams){
	std::vector<LemniType> paramTys(params, params + numParams);

	auto &&fnMap = types->fnTys[result];

	auto res = fnMap.find(paramTys);
	if(res != end(fnMap))
		return res->second.get();

	auto ptr = std::make_unique<LemniFunctionTypeT>(&types->top, result, paramTys);

	auto emplaceRes = fnMap.try_emplace(paramTys, std::move(ptr));

	return emplaceRes.first->second.get();
}
