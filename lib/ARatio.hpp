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

#ifndef LEMNI_LIB_ARATIO_HPP
#define LEMNI_LIB_ARATIO_HPP 1

#include <cstdlib>

#include "gmp.h"

#include "lemni/ARatio.h"

struct LemniARatioT{
	mpq_t val;
};

namespace {
	inline LemniARatio createARatio(){
		auto mem = std::malloc(sizeof(LemniARatioT));
		return new(mem) LemniARatioT;
	}
}

#endif // !LEMNI_LIB_ARATIO_HPP
