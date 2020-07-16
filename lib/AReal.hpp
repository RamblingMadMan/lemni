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

#ifndef LEMNI_LIB_AREAL_HPP
#define LEMNI_LIB_AREAL_HPP 1

#include "mpfr.h"
#include "arb.h"

#include "lemni/AReal.h"

struct LemniARealT{
	arb_t val;
};

namespace {
	LemniAReal createAReal(){
		auto mem = std::malloc(sizeof(LemniARealT));
		auto ret = new(mem) LemniARealT;
		arb_init(ret->val);
		return ret;
	}
}

#endif // !LEMNI_LIB_AREAL_HPP
