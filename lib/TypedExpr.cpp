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

#include "TypedExpr.hpp"

void lemniDestroyTypedExpr(LemniTypedExpr expr){ deleteTypedExpr(expr); }

LemniStr lemniTypedExprStr(LemniTypedExpr expr){ return expr->toStr(); }

LemniType lemniTypedExprType(LemniTypedExpr expr){ return expr->type(); }

LemniStr lemniTypedLValueExprId(LemniTypedLValueExpr lvalue){ return lemni::fromStdStrView(lvalue->id()); }

/********
 *
 *  Constant literal expressions
 *
 ********/

LemniTypedConstantExpr lemniTypedLiteralAsConstant(LemniTypedLiteralExpr lit){ return dynamic_cast<LemniTypedConstantExpr>(lit); }
LemniType lemniTypedConstantType(LemniTypedConstantExpr constant){ return constant->type(); }
LemniTypedLiteralExpr lemniTypedConstantBase(LemniTypedConstantExpr constant){ return constant; }
LemniTypedExpr lemniTypedConstantRoot(LemniTypedConstantExpr constant){ return constant; }

LemniTypedNumExpr lemniTypedConstantAsNum(LemniTypedConstantExpr constant){ return dynamic_cast<LemniTypedNumExpr>(constant); }
LemniType lemniTypedNumType(LemniTypedNumExpr num){ return num->type(); }
LemniTypedConstantExpr lemniTypedNumBase(LemniTypedNumExpr num){ return num; }
LemniTypedExpr lemniTypedNumRoot(LemniTypedNumExpr num){ return num; }

/********
 *
 *  Numeric literals
 *
 ********/

LemniTypedNatExpr lemniTypedNumAsNat(LemniTypedNumExpr num){ return dynamic_cast<LemniTypedNatExpr>(num); }
LemniTypedNumExpr lemniTypedNatBase(LemniTypedNatExpr n){ return n; }
LemniNatType lemniTypedNatType(LemniTypedNatExpr n){ return n->type(); }

LemniTypedANatExpr lemniCreateTypedANat(LemniTypeSet types, LemniAIntConst val){
	auto valCopy = lemniCreateAIntCopy(val);
	return newTypedExpr<LemniTypedANatExprT>(lemniTypeSetGetNat(types, 0), lemni::AInt::from(valCopy));
}

LemniTypedNat16Expr lemniCreateTypedNat16(LemniTypeSet types, LemniNat16 n16){
	return newTypedExpr<LemniTypedNat16ExprT>(lemniTypeSetGetNat(types, 16), n16);
}

LemniTypedNat32Expr lemniCreateTypedNat32(LemniTypeSet types, LemniNat32 n32){
	return newTypedExpr<LemniTypedNat32ExprT>(lemniTypeSetGetNat(types, 32), n32);
}

LemniTypedNat64Expr lemniCreateTypedNat64(LemniTypeSet types, LemniNat64 n64){
	return newTypedExpr<LemniTypedNat64ExprT>(lemniTypeSetGetNat(types, 64), n64);
}

LemniAIntConst lemniTypedANatValue(LemniTypedANatExpr n){ return n->value; }
LemniNat16 lemniTypedNat16Value(LemniTypedNat16Expr n16){ return n16->value; }
LemniNat32 lemniTypedNat32Value(LemniTypedNat32Expr n32){ return n32->value; }
LemniNat64 lemniTypedNat64Value(LemniTypedNat64Expr n64){ return n64->value; }

LemniTypedNatExpr lemniTypedANatBase(LemniTypedANatExpr n){ return n; }
LemniTypedNatExpr lemniTypedNat16Base(LemniTypedNat16Expr n16){ return n16; }
LemniTypedNatExpr lemniTypedNat32Base(LemniTypedNat32Expr n32){ return n32; }
LemniTypedNatExpr lemniTypedNat64Base(LemniTypedNat64Expr n64){ return n64; }

LemniTypedExpr lemniTypedANatRoot(LemniTypedANatExpr n){ return n; }
LemniTypedExpr lemniTypedNat16Root(LemniTypedNat16Expr n16){ return n16; }
LemniTypedExpr lemniTypedNat32Root(LemniTypedNat32Expr n32){ return n32; }
LemniTypedExpr lemniTypedNat64Root(LemniTypedNat64Expr n64){ return n64; }

LemniTypedIntExpr lemniTypedNumAsInt(LemniTypedNumExpr num){ return dynamic_cast<LemniTypedIntExpr>(num); }
LemniTypedNumExpr lemniTypedIntBase(LemniTypedIntExpr z){ return z; }
LemniIntType lemniTypedIntType(LemniTypedIntExpr z){ return z->type(); }

LemniTypedAIntExpr lemniCreateTypedAInt(LemniTypeSet types, LemniAIntConst val){
	auto valCopy = lemniCreateAIntCopy(val);
	return newTypedExpr<LemniTypedAIntExprT>(lemniTypeSetGetInt(types, 0), lemni::AInt::from(valCopy));
}

LemniTypedInt16Expr lemniCreateTypedInt16(LemniTypeSet types, LemniInt16 z16){
	return newTypedExpr<LemniTypedInt16ExprT>(lemniTypeSetGetInt(types, 16), z16);
}

LemniTypedInt32Expr lemniCreateTypedInt32(LemniTypeSet types, LemniInt32 z32){
	return newTypedExpr<LemniTypedInt32ExprT>(lemniTypeSetGetInt(types, 32), z32);
}

LemniTypedInt64Expr lemniCreateTypedInt64(LemniTypeSet types, LemniInt64 z64){
	return newTypedExpr<LemniTypedInt64ExprT>(lemniTypeSetGetInt(types, 64), z64);
}

LemniTypedAIntExpr lemniTypedIntAsAInt(LemniTypedIntExpr z){ return dynamic_cast<LemniTypedAIntExpr>(z); }
LemniTypedInt16Expr lemniTypedIntAsInt16(LemniTypedIntExpr z){ return dynamic_cast<LemniTypedInt16Expr>(z); }
LemniTypedInt32Expr lemniTypedIntAsInt32(LemniTypedIntExpr z){ return dynamic_cast<LemniTypedInt32Expr>(z); }
LemniTypedInt64Expr lemniTypedIntAsInt64(LemniTypedIntExpr z){ return dynamic_cast<LemniTypedInt64Expr>(z); }

LemniAIntConst lemniTypedAIntValue(LemniTypedAIntExpr z){ return z->value; }
LemniInt16 lemniTypedInt16Value(LemniTypedInt16Expr z16){ return z16->value; }
LemniInt32 lemniTypedInt32Value(LemniTypedInt32Expr z32){ return z32->value; }
LemniInt64 lemniTypedInt64Value(LemniTypedInt64Expr z64){ return z64->value; }

LemniTypedIntExpr lemniTypedAIntBase(LemniTypedAIntExpr z){ return z; }
LemniTypedIntExpr lemniTypedInt16Base(LemniTypedInt16Expr z16){ return z16; }
LemniTypedIntExpr lemniTypedInt32Base(LemniTypedInt32Expr z32){ return z32; }
LemniTypedIntExpr lemniTypedInt64Base(LemniTypedInt64Expr z64){ return z64; }

LemniTypedRatioExpr lemniTypedNumAsRatio(LemniTypedNumExpr num){ return dynamic_cast<LemniTypedRatioExpr>(num); }
LemniTypedNumExpr lemniTypedRatioBase(LemniTypedRatioExpr q){ return q; }
LemniRatioType lemniTypedRatioType(LemniTypedRatioExpr q){ return q->type(); }

LemniTypedARatioExpr lemniCreateTypedARatio(LemniTypeSet types, LemniARatioConst val){
	auto valCopy = lemniCreateARatioCopy(val);
	return newTypedExpr<LemniTypedARatioExprT>(lemniTypeSetGetRatio(types, 0), lemni::ARatio::from(valCopy));
}

LemniTypedRatio32Expr lemniCreateTypedRatio32(LemniTypeSet types, LemniRatio32 q32){
	return newTypedExpr<LemniTypedRatio32ExprT>(lemniTypeSetGetRatio(types, 32), q32);
}

LemniTypedRatio64Expr lemniCreateTypedRatio64(LemniTypeSet types, LemniRatio64 z64){
	return newTypedExpr<LemniTypedRatio64ExprT>(lemniTypeSetGetRatio(types, 64), z64);
}

LemniTypedRatio128Expr lemniCreateTypedRatio128(LemniTypeSet types, LemniRatio128 z128){
	return newTypedExpr<LemniTypedRatio128ExprT>(lemniTypeSetGetRatio(types, 128), z128);
}

LemniTypedARatioExpr lemniTypedRatioAsARatio(LemniTypedRatioExpr q){ return dynamic_cast<LemniTypedARatioExpr>(q); }
LemniTypedRatio32Expr lemniTypedRatioAsRatio32(LemniTypedRatioExpr q){ return dynamic_cast<LemniTypedRatio32Expr>(q); }
LemniTypedRatio64Expr lemniTypedRatioAsRatio64(LemniTypedRatioExpr q){ return dynamic_cast<LemniTypedRatio64Expr>(q); }
LemniTypedRatio128Expr lemniTypedRatioAsRatio128(LemniTypedRatioExpr q){ return dynamic_cast<LemniTypedRatio128Expr>(q); }

LemniARatioConst lemniTypedARatioValue(LemniTypedARatioExpr q){ return q->value; }
LemniRatio32 lemniTypedRatio32Value(LemniTypedRatio32Expr q32){ return q32->value; }
LemniRatio64 lemniTypedRatio64Value(LemniTypedRatio64Expr q64){ return q64->value; }
LemniRatio128 lemniTypedRatio128Value(LemniTypedRatio128Expr q128){ return q128->value; }

LemniTypedRatioExpr lemniTypedARatioBase(LemniTypedARatioExpr q){ return q; }
LemniTypedRatioExpr lemniTypedRatio32Base(LemniTypedRatio32Expr q32){ return q32; }
LemniTypedRatioExpr lemniTypedRatio64Base(LemniTypedRatio64Expr q64){ return q64; }
LemniTypedRatioExpr lemniTypedRatio128Base(LemniTypedRatio128Expr q128){ return q128; }

LemniTypedExpr lemniTypedARatioRoot(LemniTypedARatioExpr q){ return q; }
LemniTypedExpr lemniTypedRatio32Root(LemniTypedRatio32Expr q32){ return q32; }
LemniTypedExpr lemniTypedRatio64Root(LemniTypedRatio64Expr q64){ return q64; }
LemniTypedExpr lemniTypedRatio128Root(LemniTypedRatio128Expr q128){ return q128; }

LemniTypedRealExpr lemniTypedNumAsReal(LemniTypedNumExpr num){ return dynamic_cast<LemniTypedRealExpr>(num); }
LemniTypedNumExpr lemniTypedRealBase(LemniTypedRealExpr r){ return r; }
LemniRealType lemniTypedRealType(LemniTypedRealExpr r){ return r->type(); }

LemniTypedARealExpr lemniCreateTypedAReal(LemniTypeSet types, LemniARealConst val){
	auto valCopy = lemniCreateARealCopy(val);
	return newTypedExpr<LemniTypedARealExprT>(lemniTypeSetGetReal(types, 0), lemni::AReal::from(valCopy));
}

LemniTypedReal32Expr lemniCreateTypedReal32(LemniTypeSet types, LemniReal32 r32){
	return newTypedExpr<LemniTypedReal32ExprT>(lemniTypeSetGetReal(types, 32), r32);
}

LemniTypedReal64Expr lemniCreateTypedReal64(LemniTypeSet types, LemniReal64 r64){
	return newTypedExpr<LemniTypedReal64ExprT>(lemniTypeSetGetReal(types, 64), r64);
}

LemniARealConst lemniTypedARealValue(LemniTypedARealExpr r){ return r->value; }
LemniReal32 lemniTypedReal32Value(LemniTypedReal32Expr r32){ return r32->value; }
LemniReal64 lemniTypedReal64Value(LemniTypedReal64Expr r64){ return r64->value; }

LemniTypedRealExpr lemniTypedARealBase(LemniTypedARealExpr r){ return r; }
LemniTypedRealExpr lemniTypedReal32Base(LemniTypedReal32Expr r32){ return r32; }
LemniTypedRealExpr lemniTypedReal64Base(LemniTypedReal64Expr r64){ return r64; }

LemniTypedExpr lemniTypedARealRoot(LemniTypedARealExpr r){ return r; }
LemniTypedExpr lemniTypedReal32Root(LemniTypedReal32Expr r32){ return r32; }
LemniTypedExpr lemniTypedReal64Root(LemniTypedReal64Expr r64){ return r64; }

ffi_type *lemniTypeToFFI(LemniType type){
	if(lemniTypeAsUnit(type) || lemniTypeAsBottom(type)){
		return &ffi_type_void;
	}
	else if(lemniTypeAsBool(type)){
		return &ffi_type_uint8;
	}
	else if(lemniTypeAsNat(type)){
		switch(lemniTypeNumBits(type)){
			case 16: return &ffi_type_uint16;
			case 32: return &ffi_type_uint32;
			case 64: return &ffi_type_uint64;
			default: return nullptr;
		}
	}
	else if(lemniTypeAsInt(type)){
		switch(lemniTypeNumBits(type)){
			case 16: return &ffi_type_sint16;
			case 32: return &ffi_type_sint32;
			case 64: return &ffi_type_sint64;
			default: return nullptr;
		}
	}
	else if(lemniTypeAsReal(type)){
		switch(lemniTypeNumBits(type)){
			case 32: return &ffi_type_float;
			case 64: return &ffi_type_double;
			default: return nullptr;
		}
	}
	else{
		return nullptr;
	}
}
