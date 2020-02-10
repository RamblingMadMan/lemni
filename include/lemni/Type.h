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

/**
 * @defgroup Types Type checking related types and functions.
 * @{
 */

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
LemniTopType lemniNumberTypeBase(LemniNumberType num);
LemniType lemniNumberAsType(LemniNumberType num);

LemniRealType lemniTypeAsReal(LemniType type);
LemniNumberType lemniRealTypeBase(LemniRealType real);
LemniRealType lemniRealTypeAbstract(LemniRealType real);
LemniType lemniRealAsType(LemniRealType real);

LemniRatioType lemniTypeAsRatio(LemniType type);
LemniRealType lemniRatioTypeBase(LemniRatioType ratio);
LemniRatioType lemniRatioTypeAbstract(LemniRatioType ratio);
LemniType lemniRatioAsType(LemniRatioType ratio);

LemniIntType lemniTypeAsInt(LemniType type);
LemniRatioType lemniIntTypeBase(LemniIntType int_);
LemniIntType lemniIntTypeAbstract(LemniIntType int_);
LemniType lemniIntAsType(LemniIntType int_);

LemniNatType lemniTypeAsNat(LemniType type);
LemniIntType lemniNatTypeBase(LemniNatType nat);
LemniNatType lemniNatTypeAbstract(LemniNatType nat);
LemniType lemniNatAsType(LemniNatType nat);

LemniStringType lemniTypeAsString(LemniType type);
LemniTopType lemniStringTypeBase(LemniStringType str);
LemniType lemniStringAsType(LemniStringType str);

LemniStringASCIIType lemniTypeAsStringASCII(LemniType type);
LemniStringType lemniStringASCIITypeBase(LemniStringASCIIType strA);

LemniStringUTF8Type lemniTypeAsStringUTF8(LemniType type);
LemniStringASCIIType lemniStringUTF8TypeBase(LemniStringUTF8Type strU);

LemniFunctionType lemniTypeAsFunction(LemniType type);
LemniTopType lemniFunctionTypeBase(LemniFunctionType fn);
LemniType lemniFunctionTypeResult(LemniFunctionType fn);
uint32_t lemniFunctionTypeNumParams(LemniFunctionType fn);
LemniType lemniFunctionTypeParam(LemniFunctionType fn, const uint32_t idx);
LemniType lemniFunctionAsType(LemniFunctionType fn);

LemniSumType lemniTypeAsSum(LemniType type);
LemniTopType lemniSumTypeBase(LemniSumType sum);
uint32_t lemniSumTypeNumCases(LemniSumType sum);
LemniType lemniSumTypeCase(LemniSumType sum, const uint32_t idx);
LemniType lemniSumAsType(LemniSumType sum);

LemniProductType lemniTypeAsProduct(LemniType type);
LemniTopType lemniProductTypeBase(LemniProductType product);
uint32_t lemniProductTypeNumComponents(LemniProductType product);
LemniType lemniProductTypeComponent(LemniProductType product, const uint32_t idx);
LemniType lemniProductAsType(LemniProductType product);

LemniRecordType lemniTypeAsRecord(LemniType type);
LemniTopType lemniRecordTypeBase(LemniRecordType record);
uint32_t lemniRecordTypeNumFields(LemniRecordType record);
const LemniRecordTypeField *lemniRecordTypeField(LemniRecordType record, const uint32_t idx);
LemniType lemniRecordAsType(LemniRecordType record);

/**
 * @defgroup TypeSet Types and functions for containing a set of types.
 * @{
 */

/**
 * @brief Opaque type representing a set of types.
 */
typedef struct LemniTypeSetT *LemniTypeSet;

/**
 * @brief Create a new set of types.
 * @note The result must be destroyed with \ref lemniDestroyTypeSet .
 * @returns newly created type set
 */
LemniTypeSet lemniCreateTypeSet(void);

/**
 * @brief Destroy a type set previously created with \ref lemniCreateTypeSet .
 * @warning ``NULL`` must not be passed to this function.
 * @param types the type set to destroy
 */
void lemniDestroyTypeSet(LemniTypeSet types);

LemniTopType lemniTypeSetGetTop(LemniTypeSet types);
LemniBottomType lemniTypeSetGetBottom(LemniTypeSet types);
LemniUnitType lemniTypeSetGetUnit(LemniTypeSet types);
LemniBoolType lemniTypeSetGetBool(LemniTypeSet types);

LemniNumberType lemniTypeSetGetNumber(LemniTypeSet types);
LemniRealType lemniTypeSetGetReal(LemniTypeSet types, const uint32_t numBits);
LemniRatioType lemniTypeSetGetRatio(LemniTypeSet types, const uint32_t numBits);
LemniIntType lemniTypeSetGetInt(LemniTypeSet types, const uint32_t numBits);
LemniNatType lemniTypeSetGetNat(LemniTypeSet types, const uint32_t numBits);

LemniStringType lemniTypeSetGetString(LemniTypeSet types);
LemniStringASCIIType lemniTypeSetGetStringASCII(LemniTypeSet types);
LemniStringUTF8Type lemniTypeSetGetStringUTF8(LemniTypeSet types);

LemniSumType lemniTypeSetGetSum(LemniTypeSet types, LemniType *const cases, const uint32_t numCases);

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, const uint32_t numComponents);

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, const uint32_t numFields);

LemniFunctionType lemniTypeSetGetFunction(LemniTypeSet types, LemniType result, LemniType *const params, const uint32_t numParams);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
#include <climits>
#include <type_traits>

#include "Interop.h"

namespace lemni{
	using Type = LemniType;

	class TypeSet{
		public:
			TypeSet(): types(lemniCreateTypeSet()){}
			~TypeSet(){ lemniDestroyTypeSet(types); }

			LemniTypeSet handle() noexcept{ return types; }

		private:
			LemniTypeSet types;
	};

	namespace detail{
		template<typename T, typename Enable = void>
		struct CTypeGetter;

		template<>
		struct CTypeGetter<interop::Unit>{
			static LemniUnitType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetUnit(types); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniUnitAsType(t); }
		};

		template<>
		struct CTypeGetter<bool>{
			static LemniBoolType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetBool(types); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniBoolAsType(t); }
		};

		template<typename Nat>
		struct CTypeGetter<Nat, std::enable_if_t<std::is_integral_v<Nat> && std::is_unsigned_v<Nat>>>{
			static LemniNatType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetNat(types, sizeof(Nat) * CHAR_BIT); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniNatAsType(t); }
		};

		template<typename Int>
		struct CTypeGetter<Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>>>{
			static LemniIntType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetInt(types, sizeof(Int) * CHAR_BIT); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniIntAsType(t); }
		};

		template<>
		struct CTypeGetter<interop::Ratio32>{
			static LemniRatioType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetRatio(types, 32); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniRatioAsType(t); }
		};

		template<>
		struct CTypeGetter<interop::Ratio64>{
			static LemniRatioType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetRatio(types, 64); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniRatioAsType(t); }
		};

		template<>
		struct CTypeGetter<interop::Ratio128>{
			static LemniRatioType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetRatio(types, 128); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniRatioAsType(t); }
		};

		template<typename Real>
		struct CTypeGetter<Real, std::enable_if_t<std::is_floating_point_v<Real>>>{
			static LemniRealType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetReal(types, sizeof(Real) * CHAR_BIT); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniRealAsType(t); }
		};

		template<typename Ret, typename ... Params>
		struct CTypeGetter<Ret(Params...)>{
			static LemniFunctionType get(LemniTypeSet types) noexcept{
				LemniType paramTys[] = { CTypeGetter<Params>::getBare(types)... };
				return lemniTypeSetGetFunction(types, CTypeGetter<Ret>::getBare(types), paramTys, sizeof(paramTys)/sizeof(*paramTys));
			}

			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniFunctionAsType(t); }
		};
	}

	template<typename T>
	auto getCType(TypeSet &types) noexcept{ return detail::CTypeGetter<T>::get(types.handle()); }

	template<typename T>
	auto getCTypeBare(TypeSet &types) noexcept{ return detail::CTypeGetter<T>::getBare(types.handle()); }
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_TYPE_H
