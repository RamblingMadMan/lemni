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

#ifndef LEMNI_INTEROP_H
#define LEMNI_INTEROP_H 1

#include "Str.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	char _pad;
} LemniUnit;

using LemniBool = bool;

using LemniNat16 = uint16_t;
using LemniNat32 = uint32_t;
using LemniNat64 = uint64_t;

using LemniInt16 = int16_t;
using LemniInt32 = int32_t;
using LemniInt64 = int64_t;

typedef struct {
	LemniInt16 num;
	LemniNat16 den;
} LemniRatio32;

typedef struct {
	LemniInt32 num;
	LemniNat32 den;
} LemniRatio64;

typedef struct {
	LemniInt64 num;
	LemniNat64 den;
} LemniRatio128;

#ifndef __STDC_IEC_559__
#error "Floating point types must be IEEE-754 compliant"
#else
using LemniReal32 = float;
using LemniReal64 = double;
#endif

#ifdef __cplusplus
}
#ifndef LEMNI_NO_CPP
namespace lemni::interop{
	using Unit = LemniUnit;

	using Bool = LemniBool;

	using Nat16 = LemniNat16;
	using Nat32 = LemniNat32;
	using Nat64 = LemniNat64;

	using Int16 = LemniInt16;
	using Int32 = LemniInt32;
	using Int64 = LemniInt64;

	using Ratio32 = LemniRatio32;
	using Ratio64 = LemniRatio64;
	using Ratio128 = LemniRatio128;

	using Real32 = LemniReal32;
	using Real64 = LemniReal64;

	using Str = LemniStr;

	static inline Unit unit;
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_INTEROP_H
