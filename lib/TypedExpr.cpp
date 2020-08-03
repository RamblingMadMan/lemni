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

LemniStr lemniTypedExprStr(LemniTypedExpr expr){ return expr->toStr(); }

LemniType lemniTypedExprType(LemniTypedExpr expr){ return expr->type(); }

LemniStr lemniTypedLValueExprId(LemniTypedLValueExpr lvalue){ return lemni::fromStdStrView(lvalue->id()); }

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
