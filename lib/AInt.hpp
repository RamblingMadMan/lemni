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

#ifndef LEMNI_LIB_AINT_HPP
#define LEMNI_LIB_AINT_HPP 1

#include "gmp.h"

#include "lemni/AInt.h"

struct LemniAIntT{
	mpz_t val;
};

namespace {
	inline LemniAInt createAInt(){
		auto mem = std::malloc(sizeof(LemniAIntT));
		return new(mem) LemniAIntT;
	}
}

#endif // !LEMNI_LIB_AINT_HPP
