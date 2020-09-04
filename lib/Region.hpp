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

#ifndef LEMNI_LIB_REGION_HPP
#define LEMNI_LIB_REGION_HPP 1

#include <vector>

#include "lemni/Region.h"

void lemniDestroyStorage(LemniStorage storage);

struct LemniRegionT{
	~LemniRegionT(){
		if(!parent){
			for(auto s : alloced){
				lemniDestroyStorage(s);
			}
		}
	}

	LemniRegion parent;
	std::vector<LemniRegion> children;
	std::vector<LemniStorage> alloced;
};

struct LemniStorageT{
	LemniRegion owner;
	LemniNat64 alignment;
	LemniMemorySize size;
};

constexpr auto alignedSize(const LemniNat64 size, const LemniNat64 alignment) noexcept{
	auto rem = size % alignment;
	return (rem != 0) ? (size + (alignment - rem)) : size;
}

#endif // !LEMNI_LIB_REGION_HPP
