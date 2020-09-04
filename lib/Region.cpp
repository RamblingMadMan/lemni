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

#include <algorithm>
#include <bit>

#include "Region.hpp"

// TODO: possibly replace 'std::find' with binary search, depends if data needs linear structure

LemniRegion lemniCreateRegion(LemniRegion parent){
	auto mem = std::malloc(sizeof(LemniRegionT));
	auto p = new(mem) LemniRegionT;
	if(parent) parent->children.emplace_back(p);
	p->parent = parent;
	return p;
}

void lemniDestroyRegion(LemniRegion region){
	if(region->parent){
		auto it = std::find(begin(region->parent->children), end(region->parent->children), region);
		if(it != end(region->parent->children)){
			region->parent->children.erase(it);
		}
	}

	std::destroy_at(region);
	std::free(region);
}

LemniRegionConst lemniRegionParent(LemniRegionConst region){ return region->parent; }

LemniNat64 lemniRegionNumChildren(LemniRegionConst region){ return region->children.size(); }

LemniRegionConst lemniRegionChild(LemniRegionConst region, const LemniNat64 idx){
	if(idx >= region->alloced.size()) return nullptr;
	else return region->children[idx];
}

LemniNat64 lemniRegionNumStorages(LemniRegionConst region){ return region->alloced.size(); }

LemniNat64 lemniRegionMinAlignment(LemniRegionConst region, const bool queryChildren){
	auto alignment = std::numeric_limits<LemniNat64>::max();

	for(auto storage : region->alloced){
		alignment = std::min(alignment, storage->alignment);
	}

	if(queryChildren){
		for(auto child : region->children){
			alignment = std::min(alignment, lemniRegionMinAlignment(child, true));
		}
	}

	return alignment;
}

LemniNat64 lemniRegionRawSize(LemniRegionConst region, const bool queryChildren){
	LemniNat64 size = 0;

	for(auto storage : region->alloced){
		size += lemniStorageAlignedSize(storage);
	}

	if(queryChildren){
		for(auto child : region->children){
			size += lemniRegionRawSize(child, true);
		}
	}

	return size;
}

LemniNat64 lemniRegionAlignedSize(LemniRegionConst region, const bool queryChildren, const LemniNat64 alignment_){
	LemniNat64 size = 0;
	LemniNat64 alignment = alignment_;

	if(alignment == 0){
		alignment = lemniRegionMinAlignment(region, queryChildren);
	}
	else if(!std::has_single_bit(alignment)){
		alignment = std::bit_ceil(alignment);
	}

	for(auto storage : region->alloced){
		size += lemniStorageAlignedSize(storage);
		auto rem = size % alignment;
		if(rem != 0){
			size += alignment - rem;
		}
	}

	if(queryChildren){
		for(auto child : region->children){
			size += lemniRegionAlignedSize(child, true, alignment);
		}
	}

	return size;
}

LemniMemorySize lemniRegionSize(LemniRegionConst region, const bool queryChildren, const LemniNat64 alignment_){
	LemniNat64 raw = 0, aligned = 0;
	LemniNat64 alignment = alignment_;

	if(alignment == 0){
		alignment = lemniRegionMinAlignment(region, queryChildren);
	}
	else if(!std::has_single_bit(alignment)){
		alignment = std::bit_ceil(alignment);
	}

	for(auto storage : region->alloced){
		auto storageSize = lemniStorageAlignedSize(storage);

		raw += storageSize;
		aligned += storageSize;

		auto rem = aligned % alignment;
		if(rem != 0){
			aligned += alignment - rem;
		}
	}

	if(queryChildren){
		for(auto child : region->children){
			auto size = lemniRegionSize(child, true, alignment);
			raw += size.raw;
			aligned += size.aligned;
		}
	}

	return { .raw = raw, .aligned = aligned };
}

LemniStorageConst lemniRegionStorage(LemniRegionConst region, const LemniNat64 idx){
	if(idx >= region->alloced.size()) return nullptr;
	else return region->alloced[idx];
}

LemniStorage lemniRegionAlloc(LemniRegion region, const LemniNat64 size, const LemniNat64 alignment){
	auto mem = std::malloc(sizeof(LemniStorageT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniStorageT;

	p->owner = region;

	if(alignment == 0){
		p->alignment = 1;
	}
	else if(!std::has_single_bit(alignment)){
		p->alignment = std::bit_ceil(alignment);
	}
	else{
		p->alignment = alignment;
	}

	p->size = { .raw = size, .aligned = alignedSize(size, p->alignment) };

	region->alloced.emplace_back(p);

	return p;
}

void lemniDestroyStorage(LemniStorage storage){
	std::destroy_at(storage);
	std::free(storage);
}

LemniNat64 lemniStorageRawSize(LemniStorageConst storage){ return storage->size.raw; }

LemniNat64 lemniStorageAlignedSize(LemniStorageConst storage){ return storage->size.aligned; }

LemniMemorySize lemniStorageSize(LemniStorageConst storage){ return storage->size; }

LemniNat64 lemniStorageAlignment(LemniStorageConst storage){ return storage->alignment; }

LemniNat64 lemniStoragePreserve(LemniStorage storage, const LemniNat64 levels){
	if(levels == 0) return 0;

	LemniRegion originalOwner = storage->owner;
	LemniRegion owner = originalOwner;

	LemniNat64 traveled = 0;

	for(; (traveled < levels) && owner->parent; ++traveled){
		owner = owner->parent;
	}

	if(owner != originalOwner){
		lemniStorageTransfer(storage, owner);
	}

	return traveled;
}

bool lemniStorageTransfer(LemniStorage storage, LemniRegion region){
	if(!storage || !region) return false;

	auto eraseIt = std::find(begin(storage->owner->alloced), end(storage->owner->alloced), storage);
	if(eraseIt != end(storage->owner->alloced)){
		storage->owner->alloced.erase(eraseIt);
	}

	storage->owner = region;

	region->alloced.emplace_back(storage);

	return true;
}
