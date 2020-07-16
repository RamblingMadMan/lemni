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
#include <numeric>

#include "lemni/Str.h"

#include "Type.hpp"

bool LemniNatTypeT::isCastable(LemniType to) const noexcept{
	if(auto nat = lemniTypeAsNat(to)){
		return nat->numBits == 0 || numBits <= nat->numBits;
	}
	else if(auto int_ = lemniTypeAsInt(to)){
		auto natBits = std::max(int64_t(int_->numBits) - 1, 0L);
		return natBits == 0 || numBits <= natBits;
	}
	else if(auto ratio = lemniTypeAsRatio(to)){
		auto intBits = ratio->numBits / 2;
		auto natBits = std::max(int64_t(intBits - 1), 0L);
		return natBits == 0 || numBits <= natBits;
	}
	else if(auto real_ = lemniTypeAsReal(to)){
		if(numBits == 0) return real_->numBits == 0;
		else if(real_->numBits == 32 && numBits < 23) return true;
		else if(real_->numBits == 64 && numBits < 52) return true;
		else return real_->numBits == 0;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

bool LemniIntTypeT::isCastable(LemniType to) const noexcept{
	if(auto int_ = lemniTypeAsInt(to)){
		return int_->numBits == 0 || numBits <= int_->numBits;
	}
	else if(auto ratio = lemniTypeAsRatio(to)){
		auto intBits = ratio->numBits / 2;
		return intBits == 0 || numBits <= intBits;
	}
	else if(auto real_ = lemniTypeAsReal(to)){
		if(numBits == 0) return real_->numBits == 0;
		else if(real_->numBits >= 32 && numBits < 23) return true;
		else if(real_->numBits >= 64 && numBits < 52) return true;
		else return real_->numBits == 0;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

bool LemniRatioTypeT::isCastable(LemniType to) const noexcept{
	if(auto ratio = lemniTypeAsRatio(to)){
		auto intBits = ratio->numBits / 2;
		return intBits == 0 || numBits <= intBits;
	}
	else if(auto real_ = lemniTypeAsReal(to)){
		if(numBits == 0) return real_->numBits == 0;
		else if(numBits <= real_->numBits) return true;
		else return real_->numBits == 0;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

bool LemniRealTypeT::isCastable(LemniType to) const noexcept{
	if(auto real_ = lemniTypeAsReal(to)){
		if(numBits == 0) return real_->numBits == 0;
		else if(numBits <= real_->numBits) return true;
		else return real_->numBits == 0;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

bool LemniStringASCIITypeT::isCastable(LemniType to) const noexcept{
	if(auto strUtf8 = lemniTypeAsStringUTF8(to)) return true;
	else if(auto str = lemniTypeAsString(to)) return true;
	else return LemniTypeT::isCastable(to);
}

bool LemniStringUTF8TypeT::isCastable(LemniType to) const noexcept{
	if(auto str = lemniTypeAsString(to)) return true;
	else return LemniTypeT::isCastable(to);
}

bool LemniArrayTypeT::isCastable(LemniType to) const noexcept{
	if(auto arr = lemniTypeAsArray(to)){
		if(arr->elementType != elementType) return false;
		else return arr->numElements <= numElements;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

bool LemniProductTypeT::isCastable(LemniType to) const noexcept{
	if(auto prod = lemniTypeAsProduct(to)){
		if(prod->components.size() != components.size()) return false;

		for(std::size_t i = 0; i < components.size(); i++){
			auto toElem = prod->components[i];
			auto elem = components[i];
			if(!elem->isCastable(toElem)) return false;
		}

		return true;
	}
	else{
		return LemniTypeT::isCastable(to);
	}
}

LemniTypeInfo lemniEmptyTypeInfo(){
	return zeroedTypeInfo();
}

bool lemniTypeIsCastable(LemniType from, LemniType to){
	return from->isCastable(to);
}

LemniStr lemniTypeStr(LemniType type){ return {type->str.c_str(), type->str.size()}; }
LemniType lemniTypeBase(LemniType type){ return type->base; }
LemniType lemniTypeAbstract(LemniType type){ return type->abstract; }
uint32_t lemniTypeNumBits(LemniType type){ return type->numBits; }
uint64_t lemniTypeInfoIndex(LemniType type){ return type->typeInfoIdx; }

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

LemniArrayType lemniTypeAsArray(LemniType type){ return dynamic_cast<LemniArrayType>(type); }
LemniTopType lemniArrayTypeBase(LemniArrayType arr){ return reinterpret_cast<LemniTopType>(arr->base); }
LemniType lemniArrayTypeElements(LemniArrayType arr){ return arr->elementType; }
uint32_t lemniArrayTypeNumElements(LemniArrayType arr){ return arr->numElements; }
LemniType lemniArrayTypeAsType(LemniArrayType arr){ return arr; }

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

bool lemniTypeInfoHasClass(const LemniTypeInfo *info, const LemniTypeClass typeClass){ return info->typeClass & typeClass; }
bool lemniTypeInfoHasBinaryOp(const LemniTypeInfo *info, const LemniBinaryOp op){ return info->binaryOpFlags & op; }
bool lemniTypeInfoHasUnaryOp(const LemniTypeInfo *info, const LemniUnaryOp op){ return info->unaryOpFlags & op; }

bool lemniTypeInfoIsArithmetic(const LemniTypeInfo *info){
	return
			lemniTypeInfoHasClass(info, LEMNI_TYPECLASS_SCALAR) &&
			(info->info.scalar.traits & LEMNI_SCALAR_RANGE) &&
			!(info->info.scalar.traits & LEMNI_SCALAR_BOOL);
}

inline bool operator<(const LemniRecordTypeField &lhs, const LemniRecordTypeField &rhs) noexcept{
	if(lhs.type == rhs.type) return lhs.name < rhs.name;
	else return lhs.type < rhs.type;
}

std::string mangleTypeInfo(LemniTypeSet types, const LemniTypeInfo *info) noexcept;

struct LemniTypeSetT{
	LemniTypeSetT()
		: typeInfos()
		, mangledNames()
		, top(createTypeInfo(topTypeInfo()))
		, bottom(&top, createTypeInfo(bottomTypeInfo()))
		, meta(&top, createTypeInfo(metaTypeInfo()))
		, unit(&top, createTypeInfo(unitTypeInfo()))
		, bool_(&top, createTypeInfo(boolTypeInfo()))
		, number(&top, createTypeInfo(numberTypeInfo()))
		, real(&number, &real, createTypeInfo(realTypeInfo()), 0)
		, ratio(&real, &ratio, createTypeInfo(ratioTypeInfo()), 0)
		, int_(&ratio, &int_, createTypeInfo(intTypeInfo()), 0)
		, nat(&int_, &nat, createTypeInfo(natTypeInfo()), 0)
		, str(&top, createTypeInfo(strTypeInfo()))
		, strA(&str, createTypeInfo(strAsciiTypeInfo()))
		, strU(&strA, createTypeInfo(strUtf8TypeInfo()))
	{
	}

	uint64_t createTypeInfo(const LemniTypeInfo info){
		auto ret = typeInfos.size();

		typeInfos.emplace_back(std::move(info));

		return ret;
	}

	std::vector<LemniTypeInfo> typeInfos;
	std::map<uint64_t, std::string> mangledNames;
	std::vector<std::string> storedNames;

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

	struct TypeMemComp{
		bool operator()(const LemniTypeInfo &a, const LemniTypeInfo &b) const noexcept{
			return std::memcmp(&a, &b, sizeof(LemniTypeInfo)) < 0;
		}
	};

	std::vector<std::unique_ptr<LemniModuleTypeT>> moduleTys;
	std::vector<std::unique_ptr<LemniPseudoTypeT>> pseudoTys;
	//std::map<LemniTypeInfo, std::unique_ptr<LemniPseudoTypeT>, TypeMemComp> pseudoTys;

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

	std::map<std::uint64_t, std::vector<std::string>> typeStrCache;
	std::map<std::uint64_t, std::vector<std::uint64_t>> typeIndexCache;
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

inline bool hasTrait(const LemniScalarTypeInfo *info, LemniScalarTrait trait) noexcept{
	return info->traits & trait;
}

std::string mangleTypeInfo(LemniTypeSet types, const LemniTypeInfo *info) noexcept{
	std::string ret = "";

	switch(info->typeClass){
		case LEMNI_TYPECLASS_TOP:{ return "t0"; }
		case LEMNI_TYPECLASS_BOTTOM:{ return "b0"; }
		case LEMNI_TYPECLASS_META:{ return "m0"; }
		case LEMNI_TYPECLASS_PSEUDO:{ return "?"; }

		case LEMNI_TYPECLASS_SCALAR:{
			ret += "s" + std::to_string(info->info.scalar.numBits);

			if(hasTrait(&info->info.scalar, LEMNI_SCALAR_UNIT)){
				return "s0u";
			}
			else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_TEXTUAL)){
				if(hasTrait(&info->info.scalar, LEMNI_SCALAR_ASCII)){
					ret += "sa8"; // StringASCII
				}
				else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_UTF8)){
					ret += "su8"; // StringUTF8
				}
				else{
					ret += "s0"; // String
				}
			}
			else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_RANGE)){
				if(hasTrait(&info->info.scalar, LEMNI_SCALAR_NAT)){
					ret += "n0"; // Nat
				}
				else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_INT)){
					ret += "z0"; // Int
				}
				else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_RATIO)){
					ret += "q0"; // Ratio
				}
				else if(hasTrait(&info->info.scalar, LEMNI_SCALAR_REAL)){
					ret += "r0"; // Real
				}
				else{
					if(info->info.scalar.numBits == 1){
						ret += "b0"; // Bool
					}
					else{
						ret += "x0"; // Number
					}
				}
			}

			return ret;
		}

		case LEMNI_TYPECLASS_SUM:{
			ret += "u" + std::to_string(info->info.sum.numCases);

			std::vector<std::string> casesMangled;
			casesMangled.reserve(info->info.sum.numCases);

			for(size_t i = 0; i < info->info.sum.numCases; i++){
				auto mangled = lemni::toStdStr(lemniTypeSetMangleInfo(types, info->info.sum.caseTypeIndices[i]));
				auto insertIt = std::upper_bound(begin(casesMangled), end(casesMangled), mangled);
				casesMangled.insert(insertIt, std::move(mangled));
			}

			for(auto &&mangled : casesMangled){
				ret += mangled;
			}

			return ret;
		}

		case LEMNI_TYPECLASS_PRODUCT:{
			ret += "p" + std::to_string(info->info.product.numElems);

			for(size_t i = 0; i < info->info.product.numElems; i++){
				auto mangled = lemni::toStdStr(lemniTypeSetMangleInfo(types, info->info.product.elemTypeIndices[i]));
				ret += mangled;
			}

			return ret;
		}

		case LEMNI_TYPECLASS_SIGMA:{
			ret += "e" + std::to_string(info->info.sigma.numElems);

			auto mangled = lemni::toStdStr(lemniTypeSetMangleInfo(types, info->info.sigma.elemTypeIdx));
			ret += mangled;

			return ret;
		}

		case LEMNI_TYPECLASS_RECORD:{
			ret += "r" + std::to_string(info->info.record.numFields);

			for(size_t i = 0; i < info->info.record.numFields; i++){
				auto name = types->storedNames[info->info.record.fieldNameIndices[i]];
				auto mangled = lemniTypeSetMangleInfo(types, info->info.record.fieldTypeIndices[i]);
				ret += std::to_string(name.size());
				ret += name;
				ret += lemni::toStdStr(mangled);
			}

			return ret;
		}

		case LEMNI_TYPECLASS_CALLABLE:{
			ret += "c" + std::to_string(info->info.callable.numParams);
			ret += "c" + std::to_string(info->info.callable.numClosed);

			for(size_t i = 0; i < info->info.callable.numClosed; i++){
				auto closedMangled = lemniTypeSetMangleInfo(types, info->info.callable.closedTypeIndices[i]);
				ret += lemni::toStdStr(closedMangled);
			}

			for(size_t i = 0; i < info->info.callable.numParams; i++){
				auto paramMangled = lemniTypeSetMangleInfo(types, info->info.callable.paramTypeIndices[i]);
				ret += lemni::toStdStr(paramMangled);
			}

			return ret;
		}

		default: return "";
	}
}

const LemniTypeInfo *lemniTypeSetGetTypeInfo(LemniTypeSet types, LemniType type){
	return &types->typeInfos[type->typeInfoIdx];
}

const LemniTypeInfo *lemniTypeSetGetInfo(LemniTypeSet types, const uint64_t idx){
	if(idx >= types->typeInfos.size()) return nullptr;
	return &types->typeInfos[idx];
}

LemniStr lemniTypeSetMangleInfo(LemniTypeSet types, const uint64_t idx){
	if(idx >= types->typeInfos.size()) return {.ptr = nullptr, .len = 0};

	auto res = types->mangledNames.find(idx);
	if(res != end(types->mangledNames)){
		return {res->second.data(), res->second.size()};
	}

	auto mangled = mangleTypeInfo(types, &types->typeInfos[idx]);

	auto emplaceRes = types->mangledNames.try_emplace(idx, std::move(mangled));
	if(!emplaceRes.second){
		return {.ptr = nullptr, .len = 0};
	}

	return {emplaceRes.first->second.data(), emplaceRes.first->second.size()};
}

/*
const LemniTypeInfo *lemniTypeSetDemangleInfo(LemniTypeSet types, LemniStr mangled){
	auto res = std::find_if(
		begin(types->mangledNames), end(types->mangledNames),
		[mangled](const std::unordered_map<uint64_t, std::string>::value_type &val){
			return lemni::toStdStrView(mangled) == val.second;
		}
	);

	if(res != end(types->mangledNames))
		return &types->typeInfos[res->first];


}
*/

LemniTopType lemniTypeSetGetTop(LemniTypeSet types){ return &types->top; }
LemniBottomType lemniTypeSetGetBottom(LemniTypeSet types){ return &types->bottom; }

LemniModuleType lemniTypeSetGetModule(LemniTypeSet types){
	auto info = zeroedTypeInfo();
	info.typeClass |= LEMNI_TYPECLASS_MODULE;

	auto typeIdx = types->createTypeInfo(info);

	auto ptr = std::make_unique<LemniModuleTypeT>(&types->top, typeIdx);

	auto &&res = types->moduleTys.emplace_back(std::move(ptr));

	return res.get();
}

LemniPseudoType lemniTypeSetGetPseudo(LemniTypeSet types, const LemniTypeInfo usageInfo){
	auto info = zeroPadTypeInfo(usageInfo);
	info.typeClass |= LEMNI_TYPECLASS_PSEUDO;

	auto typeIdx = types->createTypeInfo(info);

	auto ptr = std::make_unique<LemniPseudoTypeT>(&types->top, typeIdx, types->pseudoTys.size());

	auto &&res = types->pseudoTys.emplace_back(std::move(ptr));

	return res.get();
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

	auto typeInfo = realTypeInfo(numBits);
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniRealTypeT>(&types->number, &types->real, typeIdx, numBits);

	auto emplaceRes = types->realTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniRatioType lemniTypeSetGetRatio(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->ratio;

	auto res = types->ratioTys.find(numBits);
	if(res != end(types->ratioTys))
		return res->second.get();

	auto typeInfo = ratioTypeInfo(numBits);
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniRatioTypeT>(lemniTypeSetGetReal(types, numBits / 2), &types->ratio, typeIdx, numBits);

	auto emplaceRes = types->ratioTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniIntType lemniTypeSetGetInt(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->int_;

	auto res = types->intTys.find(numBits);
	if(res != end(types->intTys))
		return res->second.get();

	auto typeInfo = intTypeInfo(numBits);
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniIntTypeT>(lemniTypeSetGetRatio(types, numBits * 2), &types->int_, typeIdx, numBits);

	auto emplaceRes = types->intTys.try_emplace(numBits, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniNatType lemniTypeSetGetNat(LemniTypeSet types, const uint32_t numBits){
	if(numBits == 0) return &types->nat;

	auto res = types->natTys.find(numBits);
	if(res != end(types->natTys))
		return res->second.get();

	auto typeInfo = natTypeInfo(numBits);
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniNatTypeT>(lemniTypeSetGetInt(types, numBits + 1), &types->nat, typeIdx, numBits);

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

	auto typeInfo = sigmaTypeInfo(elementType->typeInfoIdx, numElements);
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniArrayTypeT>(&types->top, typeIdx, numElements, elementType);

	auto emplaceRes = arrMap.try_emplace(numElements, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniFunctionType lemniTypeSetGetFunction(LemniTypeSet types, LemniType result, LemniType *const params, const uint32_t numParams){
	if(!params || (numParams == 0))
		return nullptr;

	std::vector<LemniType> paramTys(params, params + numParams);

	auto &&fnMap = types->fnTys[result];

	auto res = fnMap.find(paramTys);
	if(res != end(fnMap))
		return res->second.get();

	std::vector<std::uint64_t> indices;
	indices.reserve(numParams);
	std::transform(params, params + numParams, std::back_inserter(indices), [](LemniType param){ return param->typeInfoIdx; });

	auto typeInfo = functionTypeInfo(result->typeInfoIdx, numParams, indices.data());
	auto typeIdx = types->createTypeInfo(typeInfo);

	types->typeIndexCache[typeIdx] = std::move(indices);

	auto ptr = std::make_unique<LemniFunctionTypeT>(&types->top, typeIdx, result, paramTys);

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

	std::vector<std::uint64_t> indices;
	indices.reserve(fn->params.size() + numClosed);

	constexpr auto comp = [](LemniType ty) noexcept{ return ty->typeInfoIdx; };

	std::transform(begin(fn->params), end(fn->params), std::back_inserter(indices), comp);
	std::transform(closed, closed + numClosed, std::back_inserter(indices), comp);

	auto typeInfo = closureTypeInfo(fn->result->typeInfoIdx, fn->params.size(), indices.data(), numClosed, indices.data() + fn->params.size());
	auto typeIdx = types->createTypeInfo(typeInfo);

	types->typeIndexCache[typeIdx] = std::move(indices);

	auto ptr = std::make_unique<LemniClosureTypeT>(fn, typeIdx, closedTys);

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

	std::vector<uint64_t> indices;
	indices.reserve(numCases);
	std::transform(cases, cases + numCases, std::back_inserter(indices), [](LemniType t){ return t->typeInfoIdx; });

	auto typeInfo = sumTypeInfo(numCases, indices.data());
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniSumTypeT>(&types->top, typeIdx, caseTys);

	auto emplaceRes = types->sumTys.try_emplace(caseTys, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, const uint64_t numComponents){
	std::vector<LemniType> componentTys(components, components + numComponents);

	auto res = types->productTys.find(componentTys);
	if(res != end(types->productTys))
		return res->second.get();

	std::vector<uint64_t> indices;
	indices.reserve(numComponents);
	std::transform(components, components + numComponents, std::back_inserter(indices), [](LemniType t){ return t->typeInfoIdx; });

	auto typeInfo = productTypeInfo(numComponents, indices.data());
	auto typeIdx = types->createTypeInfo(typeInfo);

	auto ptr = std::make_unique<LemniProductTypeT>(&types->top, typeIdx, componentTys);

	auto emplaceRes = types->productTys.try_emplace(componentTys, std::move(ptr));

	return emplaceRes.first->second.get();
}

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, const uint64_t numFields){
	std::vector<LemniRecordTypeField> fieldVals(fields, fields + numFields);

	auto res = types->recordTys.find(fieldVals);
	if(res != end(types->recordTys))
		return res->second.get();

	std::vector<std::string> names;
	std::vector<uint64_t> indices;

	names.reserve(numFields);
	indices.resize(numFields * 2);

	std::transform(
		fields, fields + numFields, std::back_inserter(names),
		[](const LemniRecordTypeField &f){ return lemni::toStdStr(f.name); }
	);

	std::transform(
		fields, fields + numFields, std::begin(indices),
		[](const LemniRecordTypeField &f){ return f.type->typeInfoIdx; }
	);

	std::iota(std::begin(indices) + numFields, std::end(indices), 0);

	auto typeInfo = recordTypeInfo(numFields, indices.data(), indices.data() + numFields);
	auto typeIdx = types->createTypeInfo(typeInfo);

	types->typeStrCache[typeIdx] = std::move(names);
	types->typeIndexCache[typeIdx] = std::move(indices);

	auto ptr = std::make_unique<LemniRecordTypeT>(&types->top, typeIdx, fieldVals);

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

	LemniType promoteString(LemniTypeSet, LemniStringType strType, LemniType b){
		if(lemniTypeAsStringASCII(b))
			return strType;
		else if(lemniTypeAsStringUTF8(b))
			return strType;
		else if(lemniTypeAsString(b))
			return strType;
		else
			return nullptr;
	}

	LemniType promoteStringUTF8(LemniTypeSet, LemniStringUTF8Type utf8Type, LemniType b){
		if(lemniTypeAsStringASCII(b))
			return utf8Type;
		else if(lemniTypeAsStringUTF8(b))
			return utf8Type;
		else if(auto strType = lemniTypeAsString(b))
			return strType;
		else
			return nullptr;
	}

	LemniType promoteStringASCII(LemniTypeSet, LemniStringASCIIType asciiType, LemniType b){
		if(lemniTypeAsStringASCII(b))
			return asciiType;
		else if(auto utf8Type = lemniTypeAsStringUTF8(b))
			return utf8Type;
		else if(auto strType = lemniTypeAsString(b))
			return strType;
		else
			return nullptr;
	}
}

LemniType lemniTypeMakeSigned(LemniTypeSet types, LemniType type){
	const auto info = &types->typeInfos[type->typeInfoIdx];
	if(lemniTypeInfoHasClass(info, LEMNI_TYPECLASS_SCALAR) && (info->info.scalar.traits & LEMNI_SCALAR_RANGE)){
		if(auto nat = lemniTypeAsNat(type)){
			return lemniTypeSetGetInt(types, nat->numBits + 1); // add sign bit
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
	const auto aInfo = &types->typeInfos[a->typeInfoIdx];
	const auto bInfo = &types->typeInfos[b->typeInfoIdx];

	if(lemniTypeInfoHasClass(aInfo, LEMNI_TYPECLASS_SCALAR) && lemniTypeInfoHasClass(bInfo, LEMNI_TYPECLASS_SCALAR)){
		if((aInfo->info.scalar.traits & LEMNI_SCALAR_UNIT) && (bInfo->info.scalar.traits & LEMNI_SCALAR_UNIT)){
			return a;
		}
		else if((aInfo->info.scalar.traits & LEMNI_SCALAR_TEXTUAL) && (bInfo->info.scalar.traits & LEMNI_SCALAR_TEXTUAL)){
			if(auto str = lemniTypeAsString(a)) return promoteString(types, str, b);
			else if(auto utf8 = lemniTypeAsStringUTF8(a)) return promoteStringUTF8(types, utf8, b);
			else if(auto ascii = lemniTypeAsStringASCII(a)) return promoteStringASCII(types, ascii, b);
			else return a;
		}
		else if((aInfo->info.scalar.traits & LEMNI_SCALAR_RANGE) && (bInfo->info.scalar.traits & LEMNI_SCALAR_RANGE)){
			if(auto nat = lemniTypeAsNat(a)) return promoteNat(types, nat, b);
			else if(auto int_ = lemniTypeAsInt(a)) return promoteInt(types, int_, b);
			else if(auto ratio = lemniTypeAsRatio(a)) return promoteRatio(types, ratio, b);
			else if(auto real = lemniTypeAsReal(a)) return promoteReal(types, real, b);
			else return a;
		}
	}

	LemniType cases[2] = {a, b};
	return lemniTypeSetGetSum(types, cases, sizeof(cases)/sizeof(*cases));
}
