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

#include "Macros.h"
#include "Str.h"
#include "Operator.h"

/**
 * @defgroup Types Type info related types and functions.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Opaque handle to a type
 */
typedef const struct LemniTypeT *LemniType;
typedef const struct LemniCTypeT *LemniCType;

#ifdef LEMNI_CPP
struct LemniTypeT{
	virtual ~LemniTypeT() = default;

	virtual std::string_view str() const noexcept = 0;
	virtual std::string_view mangled() const noexcept = 0;

	virtual LemniType base() const noexcept = 0;
	virtual LemniType abstract() const noexcept = 0;

	virtual std::uint32_t numBits() const noexcept = 0;

	virtual std::uint64_t typeIdx() const noexcept = 0;

	virtual bool isSame(LemniType other) const noexcept = 0;
	virtual bool isCastable(LemniType to) const noexcept = 0;
};

struct LemniCTypeT: public LemniTypeT{};
#endif

typedef const struct LemniPseudoTypeT *LemniPseudoType;
typedef const struct LemniErrorTypeT *LemniErrorType;
typedef const struct LemniModuleTypeT *LemniModuleType;

typedef const struct LemniTopTypeT *LemniTopType;
typedef const struct LemniBottomTypeT *LemniBottomType;
typedef const struct LemniMetaTypeT *LemniMetaType;
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
typedef const struct LemniClosureTypeT *LemniClosureType;

typedef const struct LemniSumTypeT *LemniSumType;
typedef const struct LemniProductTypeT *LemniProductType;
typedef const struct LemniSigmaTypeT *LemniSigmaType;

typedef const struct LemniArrayTypeT *LemniArrayType;

typedef const struct LemniRecordTypeT *LemniRecordType;

typedef struct {
	LemniStr name;
	LemniType type;
} LemniRecordTypeField;

LEMNI_BITFLAG_ENUM_T(LemniTypeClass, LEMNI_TYPECLASS,
	EMPTY,
	META,
	PSEUDO,
	MODULE,
	SCALAR,
	CALLABLE,
	SUM,
	PRODUCT,
	RECORD,
	SIGMA,
	TOP,
	BOTTOM
);

LEMNI_BITFLAG_ENUM_T(LemniScalarTrait, LEMNI_SCALAR,
	UNIT, RANGE, TEXTUAL,
	BOOL, NAT, INT, RATIO, REAL,
	ASCII, UTF8
);

struct LemniTypeInfoT;

typedef struct {
	uint32_t numBits;
	uint32_t traits;
} LemniScalarTypeInfo;

typedef struct {
	uint64_t numCases;
	const uint64_t *caseTypeIndices;
} LemniSumTypeInfo;

typedef struct {
	uint64_t numElems;
	const uint64_t *elemTypeIndices;
} LemniProductTypeInfo;

typedef struct {
	uint64_t numElems;
	uint64_t elemTypeIdx;
} LemniSigmaTypeInfo;

typedef struct {
	uint64_t numFields;
	const uint64_t *fieldTypeIndices;
	const uint64_t *fieldNameIndices;
} LemniRecordTypeInfo;

typedef struct {
	uint64_t resultTypeIdx;
	uint32_t numParams;
	uint32_t numClosed;
	const uint64_t *paramTypeIndices;
	const uint64_t *closedTypeIndices;
} LemniCallableTypeInfo;

typedef struct LemniTypeInfoT{
	uint32_t binaryOpFlags;
	uint32_t unaryOpFlags;
	uint32_t typeClass;
	union TypeInfoUnion{
		LemniScalarTypeInfo scalar;
		LemniSumTypeInfo sum;
		LemniProductTypeInfo product;
		LemniSigmaTypeInfo sigma;
		LemniRecordTypeInfo record;
		LemniCallableTypeInfo callable;
		uint8_t bytes[32];
	} info;
} LemniTypeInfo;

LemniTypeInfo lemniEmptyTypeInfo();

/**
 * @brief Check if a type is castable to another.
 * @param from the type to cast from
 * @param to the type to cast to
 * @returns whether the conversion would be lossless
 */
bool lemniTypeIsCastable(LemniType from, LemniType to);

/**
 * @brief Check if two types are the same.
 * @param a first type
 * @param b second type
 * @returns whether the types are equivalent
 */
bool lemniTypeIsSame(LemniType a, LemniType b);

LemniStr lemniTypeStr(LemniType type);
LemniStr lemniTypeMangled(LemniType type);
LemniType lemniTypeBase(LemniType type);
LemniType lemniTypeAbstract(LemniType type);
uint32_t lemniTypeNumBits(LemniType type);
uint64_t lemniTypeInfoIndex(LemniType type);

LemniTopType lemniTypeAsTop(LemniType type);
LemniType lemniTopAsType(LemniTopType top);

LemniBottomType lemniTypeAsBottom(LemniType type);
LemniTopType lemniBottomTypeBase(LemniBottomType bottom);
LemniType lemniBottomAsType(LemniBottomType bottom);

LemniErrorType lemniTypeAsError(LemniType type);
LemniTopType lemniErrorTypeBase(LemniErrorType error);
LemniType lemniErrorAsType(LemniErrorType error);
LemniType lemniErrorResult(LemniErrorType error);

LemniModuleType lemniTypeAsModule(LemniType type);
LemniTopType lemniModuleTypeBase(LemniModuleType mod);
LemniType lemniModuleAsType(LemniModuleType mod);

LemniPseudoType lemniTypeAsPseudo(LemniType type);
LemniTopType lemniPseudoTypeBase(LemniPseudoType pseudo);
LemniType lemniPseudoAsType(LemniPseudoType pseudo);

LemniMetaType lemniTypeAsMeta(LemniType type);
LemniTopType lemniMetaTypeBase(LemniMetaType meta);
LemniType lemniMetaAsType(LemniMetaType meta);
LemniTypeInfo lemniMetaTypeInfo(LemniMetaType meta);

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

LemniArrayType lemniTypeAsArray(LemniType type);
LemniTopType lemniArrayTypeBase(LemniArrayType arr);
LemniType lemniArrayTypeElements(LemniArrayType arr);
uint32_t lemniArrayTypeNumElements(LemniArrayType arr);
LemniType lemniArrayTypeAsType(LemniArrayType arr);

LemniFunctionType lemniTypeAsFunction(LemniType type);
LemniTopType lemniFunctionTypeBase(LemniFunctionType fn);
LemniType lemniFunctionTypeResult(LemniFunctionType fn);
uint32_t lemniFunctionTypeNumParams(LemniFunctionType fn);
LemniType lemniFunctionTypeParam(LemniFunctionType fn, const uint32_t idx);
LemniType lemniFunctionAsType(LemniFunctionType fn);

LemniClosureType lemniTypeAsClosure(LemniType type);
LemniFunctionType lemniClosureTypeBase(LemniClosureType closure);
uint32_t lemniClosureTypeNumClosed(LemniClosureType closure);
LemniType lemniClosureTypeClosed(LemniClosureType closure, const uint32_t idx);
LemniType lemniClosureAsType(LemniClosureType closure);

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

bool lemniTypeInfoHasClass(const LemniTypeInfo *info, const LemniTypeClass typeClass);
bool lemniTypeInfoHasBinaryOp(const LemniTypeInfo *info, const LemniBinaryOp op);
bool lemniTypeInfoHasUnaryOp(const LemniTypeInfo *info, const LemniUnaryOp op);
bool lemniTypeInfoIsArithmetic(const LemniTypeInfo *info);

/**
 * @}
 */

/**
 * @defgroup TypeSet Types and functions for containing a set of types.
 * @{
 */

/**
 * @brief Opaque type representing a set of types.
 */
typedef struct LemniTypeSetT *LemniTypeSet;
typedef const struct LemniTypeSetT *LemniTypeSetConst;

typedef size_t(*LemniIOWriteCB)(void *user, const void *ptr, const size_t len);
typedef size_t(*LemniIOReadCB)(void *user, void *ptr, size_t len);

typedef struct LemniIOT{
	LemniIOWriteCB writeCb;
	LemniIOReadCB readCb;
	void *user;
} LemniIO;

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

/**
 * @brief Serialize a typeset.
 * @warning ``NULL`` must not be passed to this function.
 * @param types the type set to serialize
 * @param io the IO stream to serialize to
 */
void lemniSerializeTypeSet(LemniTypeSet types, LemniIO *io);

void lemniTypeSetSerializeInfo(LemniTypeSet types, LemniIO *io, const LemniTypeInfo *info);

const LemniTypeInfo *lemniTypeSetGetTypeInfo(LemniTypeSet types, LemniType type);
const LemniTypeInfo *lemniTypeSetGetInfo(LemniTypeSet types, const uint64_t idx);
LemniStr lemniTypeSetMangleInfo(LemniTypeSet types, const uint64_t idx);
const LemniTypeInfo *lemniTypeSetDemangleInfo(LemniTypeSet types, LemniStr mangled);

/**
 * @brief Get the top type.
 * @param types type set to query
 * @returns top type
 */
LemniTopType lemniTypeSetGetTop(LemniTypeSet types);

/**
 * @brief Get the bottom type.
 * @param types type set to query
 * @returns bottom type
 */
LemniBottomType lemniTypeSetGetBottom(LemniTypeSet types);

/**
 * @brief Get a unique module type.
 * @param types type set to use
 * @returns new module type
 */
LemniModuleType lemniTypeSetGetModule(LemniTypeSet types);

/**
 * @brief Get a unique pseudo type.
 * @param types type set to use
 * @param usageInfo how values of the type should be used
 * @returns new pseudo type
 */
LemniPseudoType lemniTypeSetGetPseudo(LemniTypeSet types, const LemniTypeInfo usageInfo);

/**
 * @brief Get the meta type.
 * @param types type set to query
 * @returns meta type
 */
LemniMetaType lemniTypeSetGetMeta(LemniTypeSet types);

/**
 * @brief Get the unit type.
 * @param types type set to query
 * @returns unit type
 */
LemniUnitType lemniTypeSetGetUnit(LemniTypeSet types);

/**
 * @brief Get the boolean type.
 * @param types type set to query
 * @returns boolean type
 */
LemniBoolType lemniTypeSetGetBool(LemniTypeSet types);

/**
 * @brief Get the number type.
 * @param types type set to query
 * @returns number type
 */
LemniNumberType lemniTypeSetGetNumber(LemniTypeSet types);

/**
 * @brief Get a sized real type.
 * @param types type set to use
 * @param numBits number of bits or 0 for arbitrary precision
 * @returns real type
 */
LemniRealType lemniTypeSetGetReal(LemniTypeSet types, const uint32_t numBits);

/**
 * @brief Get a sized rational type.
 * @param types type set to use
 * @param numBits number of bits or 0 for arbitrary precision
 * @returns rational type
 */
LemniRatioType lemniTypeSetGetRatio(LemniTypeSet types, const uint32_t numBits);

/**
 * @brief Get a sized integer type.
 * @param types type set to use
 * @param numBits number of bits or 0 for arbitrary precision
 * @returns integer type
 */
LemniIntType lemniTypeSetGetInt(LemniTypeSet types, const uint32_t numBits);

/**
 * @brief Get a sized natural type.
 * @param types type set to use
 * @param numBits number of bits or 0 for arbitrary precision
 * @returns natural type
 */
LemniNatType lemniTypeSetGetNat(LemniTypeSet types, const uint32_t numBits);

/**
 * @brief Get the string type.
 * @param types type set to query
 * @returns string type
 */
LemniStringType lemniTypeSetGetString(LemniTypeSet types);

/**
 * @brief Get the ASCII string type.
 * @param types type set to query
 * @returns ASCII string type
 */
LemniStringASCIIType lemniTypeSetGetStringASCII(LemniTypeSet types);

/**
 * @brief Get the UTF8 string type.
 * @param types type set to query
 * @returns UTF8 string type
 */
LemniStringUTF8Type lemniTypeSetGetStringUTF8(LemniTypeSet types);

LemniArrayType lemniTypeSetGetArray(LemniTypeSet types, const uint64_t numElements, LemniType elementType);

LemniFunctionType lemniTypeSetGetFunction(LemniTypeSet types, LemniType result, LemniType *const params, const uint32_t numParams);

LemniClosureType lemniTypeSetGetClosure(LemniTypeSet types, LemniFunctionType fn, LemniType *const closed, const uint64_t numClosed);

LemniSumType lemniTypeSetGetSum(LemniTypeSet types, LemniType *const cases, const uint64_t numCases);

LemniProductType lemniTypeSetGetProduct(LemniTypeSet types, LemniType *const components, const uint64_t numComponents);

LemniRecordType lemniTypeSetGetRecord(LemniTypeSet types, const LemniRecordTypeField *const fields, const uint64_t numFields);

/**
 * @}
 */

/**
 * @defgroup TypeManipulation Type manipulation related functions
 * @{
 */

/**
 * @brief Get the signed version of a numeric type (``Nat -> Int``).
 * @note Natural types will be promoted to the next power of 2 bitwidth to accomodate 2's complement representation.
 * @param types type set to get types from
 * @param type the type to make signed
 * @returns the signed version of the type, or ``NULL`` if \p type is non-numeric
 */
LemniType lemniTypeMakeSigned(LemniTypeSet types, LemniType type);

LemniType lemniTypePromote(LemniTypeSet types, LemniType a, LemniType b);

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
	using RecordTypeField = LemniRecordTypeField;

	class TypeSet{
		public:
			TypeSet(): types(lemniCreateTypeSet()){}
			~TypeSet(){ lemniDestroyTypeSet(types); }

			operator LemniTypeSet() noexcept{ return types; }
			operator LemniTypeSetConst() const noexcept{ return types; }

			LemniTypeSet handle() noexcept{ return types; }
			LemniTypeSetConst handle() const noexcept{ return types; }

			LemniPseudoType pseudo(const LemniTypeInfo usageInfo) const noexcept{
				return lemniTypeSetGetPseudo(types, usageInfo);
			}

			LemniTopType top() const noexcept{ return lemniTypeSetGetTop(types); }
			LemniBottomType bottom() const noexcept{ return lemniTypeSetGetBottom(types); }
			LemniMetaType meta() const noexcept{ return lemniTypeSetGetMeta(types); }
			LemniUnitType unit() const noexcept{ return lemniTypeSetGetUnit(types); }
			LemniBoolType bool_() const noexcept{ return lemniTypeSetGetBool(types); }

			LemniNumberType number() const noexcept{ return lemniTypeSetGetNumber(types); }
			LemniNatType natural(const uint32_t numBits = 0) const noexcept{ return lemniTypeSetGetNat(types, numBits); }
			LemniIntType integer(const uint32_t numBits = 0) const noexcept{ return lemniTypeSetGetInt(types, numBits); }
			LemniRatioType rational(const uint32_t numBits = 0) const noexcept{ return lemniTypeSetGetRatio(types, numBits); }
			LemniRealType real(const uint32_t numBits = 0) const noexcept{ return lemniTypeSetGetReal(types, numBits); }

			LemniStringType string() const noexcept{ return lemniTypeSetGetString(types); }
			LemniStringASCIIType stringASCII() const noexcept{ return lemniTypeSetGetStringASCII(types); }
			LemniStringUTF8Type stringUTF8() const noexcept{ return lemniTypeSetGetStringUTF8(types); }

			LemniArrayType array(const uint64_t numElements, Type elementType) const noexcept{ return lemniTypeSetGetArray(types, numElements, elementType); }

			LemniFunctionType function(Type resultType, Type *const paramTypes, const uint32_t numParams) const noexcept{
				return lemniTypeSetGetFunction(types, resultType, paramTypes, numParams);
			}

			LemniClosureType closure(LemniFunctionType fnType, Type *const closedTypes, const uint32_t numClosed) const noexcept{
				return lemniTypeSetGetClosure(types, fnType, closedTypes, numClosed);
			}

			LemniSumType sum(Type *const caseTypes, const uint32_t numCases) const noexcept{ return lemniTypeSetGetSum(types, caseTypes, numCases); }

			LemniProductType product(Type *const componentTypes, const uint32_t numComponents) const noexcept{
				return lemniTypeSetGetProduct(types, componentTypes, numComponents);
			}

			LemniRecordType record(const RecordTypeField *const fields, const uint32_t numFields) const noexcept{
				return lemniTypeSetGetRecord(types, fields, numFields);
			}

		private:
			LemniTypeSet types;
	};

	namespace detail{
		template<typename T, typename Enable = void>
		struct CTypeGetter;

		template<typename Unit>
		struct CTypeGetter<Unit, std::enable_if_t<std::is_same_v<LemniUnit, Unit>>>{
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

		template<>
		struct CTypeGetter<interop::Str>{
			static LemniStringType get(LemniTypeSet types) noexcept{ return lemniTypeSetGetString(types); }
			static LemniType getBare(LemniTypeSet types) noexcept{ auto t = get(types); return lemniStringAsType(t); }
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
	auto getCType(LemniTypeSet types) noexcept{ return detail::CTypeGetter<T>::get(types); }

	template<typename T>
	auto getCType(TypeSet &types) noexcept{ return detail::CTypeGetter<T>::get(types.handle()); }

	template<typename T>
	auto getCTypeBare(LemniTypeSet types) noexcept{ return detail::CTypeGetter<T>::getBare(types); }

	template<typename T>
	auto getCTypeBare(TypeSet &types) noexcept{ return detail::CTypeGetter<T>::getBare(types.handle()); }
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

/**
 * @}
 */

#endif // !LEMNI_TYPE_H
