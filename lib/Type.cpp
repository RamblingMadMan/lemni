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

#include "Type.hpp"

LemniType lemniTypeBase(LemniType type){ return type->base; }
LemniType lemniTypeAbstract(LemniType type){ return type->abstract; }
uint32_t lemniTypeNumBits(LemniType type){ return type->numBits; }

LemniTopType lemniTypeAsTop(LemniType type){ return dynamic_cast<LemniTopType>(type); }
LemniType lemniTopAsType(LemniTopType top){ return top; }

LemniBottomType lemniTypeAsBottom(LemniType type){ return dynamic_cast<LemniBottomType>(type); }
LemniTopType lemniBottomTypeBase(LemniBottomType bottom){ return reinterpret_cast<LemniTopType>(bottom->base); }
LemniType lemniBottomAsType(LemniBottomType bottom){ return bottom; }

LemniPseudoType lemniTypeAsPseudo(LemniType type){ return dynamic_cast<LemniPseudoType>(type); }
LemniTopType lemniPseudoTypeBase(LemniPseudoType pseudo){ return reinterpret_cast<LemniTopType>(pseudo->base); }
LemniType lemniPseudoAsType(LemniPseudoType pseudo){ return pseudo; }

LemniMetaType lemniTypeAsMeta(LemniType type){ return dynamic_cast<LemniMetaType>(type); }
LemniTopType lemniMetaTypeBase(LemniMetaType meta){ return reinterpret_cast<LemniTopType>(meta->base); }
LemniType lemniMetaAsType(LemniMetaType meta){ return meta; }

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

LemniClosureType lemniTypeAsClosure(LemniType type){ return dynamic_cast<LemniClosureType>(type); }
LemniFunctionType lemniClosureTypeBase(LemniClosureType closure){ return reinterpret_cast<LemniFunctionType>(closure->base); }
uint32_t lemniClosureTypeNumClosed(LemniClosureType closure){ return static_cast<uint32_t>(closure->closed.size()); }
LemniType lemniClosureTypeClosed(LemniClosureType closure, const uint32_t idx){ return closure->closed[idx]; }
LemniType lemniClosureAsType(LemniClosureType closure){ return closure; }

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

uint32_t lemniTypeHasCategories(LemniType type, const uint32_t bitmask){
	return type->categoryMask & bitmask;
}

inline bool operator<(const LemniRecordTypeField &lhs, const LemniRecordTypeField &rhs) noexcept{
	if(lhs.type == rhs.type) return lhs.name < rhs.name;
	else return lhs.type < rhs.type;
}

struct LemniTypeSetT{
	LemniTypeSetT()
		: top()
		, bottom(&top)
		, meta(&top)
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
	LemniMetaTypeT meta;
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

	std::map<uint32_t, std::unique_ptr<LemniPseudoTypeT>> pseudoTys;

	std::map<uint32_t, std::unique_ptr<LemniRealTypeT>> realTys;
	std::map<uint32_t, std::unique_ptr<LemniRatioTypeT>> ratioTys;
	std::map<uint32_t, std::unique_ptr<LemniIntTypeT>> intTys;
	std::map<uint32_t, std::unique_ptr<LemniNatTypeT>> natTys;

	std::map<LemniType, std::map<uint64_t, std::unique_ptr<LemniArrayTypeT>>> arrTys;

	std::map<LemniType, std::map<std::vector<LemniType>, std::unique_ptr<LemniFunctionTypeT>>> fnTys;
	std::map<LemniFunctionType, std::map<std::vector<LemniType>, std::unique_ptr<LemniClosureTypeT>>> closureTys;

	std::map<std::vector<LemniType>, std::unique_ptr<LemniSumTypeT>> sumTys;
	std::map<std::vector<LemniType>, std::unique_ptr<LemniProductTypeT>> productTys;
	std::map<std::vector<LemniRecordTypeField>, std::unique_ptr<LemniRecordTypeT>> recordTys;
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

LemniPseudoType lemniTypeSetGetPseudo(LemniTypeSet types, const uint32_t categoryMask){
	auto res = types->pseudoTys.find(categoryMask);
	if(res != end(types->pseudoTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniPseudoTypeT>(&types->top, categoryMask);

	auto emplaceRes = types->pseudoTys.try_emplace(categoryMask, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniMetaType lemniTypeSetGetMeta(LemniTypeSet types){ return &types->meta; }

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

LemniArrayType lemniTypeSetGetArray(LemniTypeSet types, const uint64_t numElements, LemniType elementType){
	auto &&arrMap = types->arrTys[elementType];

	auto res = arrMap.find(numElements);
	if(res != end(arrMap))
		return res->second.get();

	auto ptr = std::make_unique<LemniArrayTypeT>(&types->top, numElements, elementType);

	auto emplaceRes = arrMap.try_emplace(numElements, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniFunctionType lemniTypeSetGetFunction(LemniTypeSet types, LemniType result, LemniType *const params, const uint32_t numParams){
	std::vector<LemniType> paramTys(params, params + numParams);

	auto &&fnMap = types->fnTys[result];

	auto res = fnMap.find(paramTys);
	if(res != end(fnMap))
		return res->second.get();

	auto ptr = std::make_unique<LemniFunctionTypeT>(&types->top, result, paramTys);

	auto emplaceRes = fnMap.try_emplace(std::move(paramTys), std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniClosureType lemniTypeSetGetClosure(LemniTypeSet types, LemniFunctionType fn, LemniType *const closed, const uint64_t numClosed){
	std::vector<LemniType> closedTys(closed, closed + numClosed);
	std::sort(begin(closedTys), end(closedTys));

	auto &&closureMap = types->closureTys[fn];

	auto res = closureMap.find(closedTys);
	if(res != end(closureMap))
		return res->second.get();

	auto ptr = std::make_unique<LemniClosureTypeT>(fn, closedTys);

	auto emplaceRes = closureMap.try_emplace(std::move(closedTys), std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniSumType lemniTypeSetGetSum(LemniTypeSet types, LemniType *const cases, const uint64_t numCases){
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

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, const uint64_t numComponents){
	std::vector<LemniType> componentTys(components, components + numComponents);

	auto res = types->productTys.find(componentTys);
	if(res != end(types->productTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniProductTypeT>(&types->top, componentTys);

	auto emplaceRes = types->productTys.try_emplace(componentTys, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, const uint64_t numFields){
	std::vector<LemniRecordTypeField> fieldVals(fields, fields + numFields);

	auto res = types->recordTys.find(fieldVals);
	if(res != end(types->recordTys))
		return res->second.get();

	auto ptr = std::make_unique<LemniRecordTypeT>(&types->top, fieldVals);

	auto emplaceRes = types->recordTys.try_emplace(fieldVals, std::move(ptr));

	return emplaceRes.first->second.get();
}

namespace {
	uint32_t nextPow2(uint32_t v){
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	LemniType promoteNat(LemniTypeSet types, LemniNatType a, LemniType b){
		auto aBits = a->numBits;
		auto bBits = b->numBits;

		if(auto nat = lemniTypeAsNat(b)){
			if(aBits == 0) return a;
			else if(bBits == 0) return b;
			else if(aBits > bBits) return a;
			else return b;
		}
		else if(auto int_ = lemniTypeAsInt(b)){
			if(aBits == 0) return lemniTypeSetGetInt(types, 0);
			else if(bBits == 0) return b;
			else if(aBits > bBits) return lemniTypeSetGetInt(types, nextPow2(aBits));
			else return b;
		}
		else if(auto ratio = lemniTypeAsRatio(b)){
			if(aBits == 0) return lemniTypeSetGetRatio(types, 0);
			else if(bBits == 0) return b;
			else if((bBits / 2) < (aBits + 1)) return lemniTypeSetGetRatio(types, nextPow2(bBits));
			else return b;
		}
		else if(auto real = lemniTypeAsReal(b)){
			if(aBits == 0) return lemniTypeSetGetReal(types, 0);
			else if(bBits == 0) return b;
			else if(bBits == 32){
				if(aBits >= 24) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
			else if(bBits == 64){
				if(aBits >= 53) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
		}

		return nullptr;
	}

	LemniType promoteInt(LemniTypeSet types, LemniIntType a, LemniType b){
		auto aBits = a->numBits;
		auto bBits = b->numBits;

		if(auto nat = lemniTypeAsNat(b)){
			if(aBits == 0) return a;
			else if(bBits == 0) return lemniTypeSetGetInt(types, 0);
			else if(bBits > aBits) return lemniTypeSetGetInt(types, nextPow2(bBits));
			else return a;
		}
		else if(auto int_ = lemniTypeAsInt(b)){
			if(aBits == 0) return a;
			else if(bBits == 0) return b;
			else if(aBits > bBits) return a;
			else return b;
		}
		else if(auto ratio = lemniTypeAsRatio(b)){
			if(aBits == 0) return lemniTypeSetGetRatio(types, 0);
			else if(bBits == 0) return b;
			else if(aBits > (bBits / 2)) return lemniTypeSetGetRatio(types, nextPow2(bBits));
			else return b;
		}
		else if(auto real = lemniTypeAsReal(b)){
			if(aBits == 0) return lemniTypeSetGetReal(types, 0);
			else if(bBits == 0) return b;
			else if(bBits == 32){
				if(aBits >= 24) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
			else if(bBits == 64){
				if(aBits >= 53) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
		}

		return nullptr;
	}

	LemniType promoteRatio(LemniTypeSet types, LemniRatioType a, LemniType b){
		auto aBits = a->numBits / 2;
		auto bBits = b->numBits;

		if(auto real = lemniTypeAsReal(b)){
			if(bBits == 32){
				if(aBits == 0) return lemniTypeSetGetReal(types, 0);
				else if(aBits >= 24) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
			else if(bBits == 64){
				if(aBits == 0) return lemniTypeSetGetReal(types, 0);
				else if(aBits >= 53) return lemniTypeSetGetReal(types, nextPow2(bBits));
				else return b;
			}
			else if(bBits == 0) return b;
		}
		else if(aBits == 0){
			return a;
		}
		else if(auto nat = lemniTypeAsNat(b)){
			if(bBits == 0) return lemniTypeSetGetRatio(types, 0);
			else if(aBits < (bBits + 1)) return lemniTypeSetGetRatio(types, nextPow2(aBits));
			else return a;
		}
		else if(auto int_ = lemniTypeAsInt(b)){
			if(bBits == 0) return lemniTypeSetGetRatio(types, 0);
			else if(bBits > aBits) return lemniTypeSetGetRatio(types, bBits * 2);
			else return a;
		}
		else if(auto ratio = lemniTypeAsRatio(b)){
			if(bBits == 0) return b;
			else if(a->numBits > bBits) return a;
			else return b;
		}

		return nullptr;
	}

	LemniType promoteReal(LemniTypeSet types, LemniRealType a, LemniType b){
		auto aBits = a->numBits;
		auto bBits = b->numBits;

		if(aBits == 0){
			return a;
		}
		else if(bBits == 0){
			return lemniTypeSetGetReal(types, 0);
		}
		else if(auto nat = lemniTypeAsNat(b)){
			if(aBits == 32){
				if(bBits >= 24) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
			else if(aBits == 64){
				if(bBits >= 53) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
		}
		else if(auto int_ = lemniTypeAsInt(b)){
			if(aBits == 32){
				if((bBits + 1) >= 24) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
			else if(aBits == 64){
				if((bBits + 1) >= 53) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
		}
		else if(auto ratio = lemniTypeAsInt(b)){
			if(aBits == 32){
				if((bBits / 2) >= 24) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
			else if(aBits == 64){
				if((bBits / 2) >= 53) return lemniTypeSetGetReal(types, nextPow2(aBits));
				else return a;
			}
		}
		else if(auto real = lemniTypeAsReal(b)){
			if(aBits > bBits) return a;
			else return b;
		}

		return nullptr;
	}
}

LemniType lemniTypeMakeSigned(LemniTypeSet types, LemniType type){
	if(lemniTypeIsArithmetic(type)){
		if(auto nat = lemniTypeAsNat(type)){
			return lemniTypeSetGetInt(types, nat->numBits > 0 ? nextPow2(nat->numBits) : 0);
		}
		else{
			return type;
		}
	}
	else{
		return nullptr;
	}
}

LemniType lemniTypePromote(LemniTypeSet types, LemniType a, LemniType b){
	if(lemniTypeIsScalar(a) && lemniTypeIsScalar(b)){
		if(lemniTypeIsArithmetic(a) && lemniTypeIsArithmetic(b)){
			if(auto nat = lemniTypeAsNat(a)) return promoteNat(types, nat, b);
			else if(auto int_ = lemniTypeAsInt(a)) return promoteInt(types, int_, b);
			else if(auto ratio = lemniTypeAsRatio(a)) return promoteRatio(types, ratio, b);
			else if(auto real = lemniTypeAsReal(a)) return promoteReal(types, real, b);
			else return a;
		}
		else{
			return nullptr;
		}
	}
	else{
		return nullptr;
	}
}
