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

#include <cstdlib>

#include <vector>

#include "lemni/memcheck.h"

#include "TypedExpr.hpp"

struct LemniMemCheckStateT{
	~LemniMemCheckStateT(){
		if(!global){
			for(auto region : regions){
				lemniDestroyRegion(region);
			}
		}
	}

	LemniRegion global;
	std::vector<LemniRegion> regions;
	std::map<LemniTypedExpr, LemniRegion> regionMap;
	std::map<LemniTypedExpr, LemniStorage> storageMap;
};

LemniMemCheckState lemniCreateMemCheckState(LemniRegion global){
	auto mem = std::malloc(sizeof(LemniMemCheckStateT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniMemCheckStateT;

	p->global = global;

	return p;
}

void lemniDestroyMemCheckState(LemniMemCheckState state){
	std::destroy_at(state);
	std::free(state);
}

LemniMemCheckResult lemniMemCheck(LemniMemCheckState state, LemniTypedExpr expr){
	if(!expr){
		LemniMemCheckResult res;
		res.hasError = false;
		res.res = { .expr = nullptr, .size = { 0, 0 } };
		return res;
	}
	else if(!state){
		LemniMemCheckResult res;
		res.hasError = true;
		res.error = { .msg = LEMNICSTR("NULL state passed") };
		return res;
	}
	else{
		return expr->memcheck(state);
	}
}

namespace {
	LemniNat64 calcSize(LemniType type){
		auto numBits = lemniTypeNumBits(type);
		return numBits / 8;
	}
}

LemniMemCheckResult LemniTypedExprT::memcheck(LemniMemCheckState state) const noexcept{
	auto region = state->global;
	if(!region){
		region = lemniCreateRegion(nullptr);
		state->regions.insert(
			std::upper_bound(begin(state->regions), end(state->regions), region),
			region
		);
	}

	const auto size = calcSize(type());

	auto storage = lemniRegionAlloc(region, size, size);

	state->regionMap[this] = region;
	state->storageMap[this] = storage;

	LemniMemCheckResult res;

	res.hasError = false;
	res.res = {
		.expr = this,
		.size = lemniStorageSize(storage)
	};

	return res;
}

LemniMemCheckResult LemniTypedLambdaExprT::memcheck(LemniMemCheckState state) const noexcept{
	if(isPseudo){
		LemniMemCheckResult res;
		res.hasError = true;
		res.error = { .msg = LEMNICSTR("can not memory check pseudo functions") };
		return res;
	}

	auto it = state->regionMap.find(this);
	if(it != end(state->regionMap)){
		LemniMemCheckResult res;
		res.hasError = false;
		res.res = { .expr = this, .size = lemniRegionSize(it->second, true, 0) };
		return res;
	}

	auto fnRegion = lemniCreateRegion(state->global);
	state->regions.insert(
		std::upper_bound(begin(state->regions), end(state->regions), fnRegion),
		fnRegion
	);

	auto paramRegion = lemniCreateRegion(fnRegion);
	auto bodyRegion = lemniCreateRegion(fnRegion);

	auto paramState = lemniCreateMemCheckState(paramRegion);

	for(auto param : params){
		auto paramRes = param->memcheck(paramState);
		if(paramRes.hasError){
			lemniDestroyMemCheckState(paramState);
			return paramRes;
		}
	}

	lemniDestroyMemCheckState(paramState);

	auto bodyState = lemniCreateMemCheckState(bodyRegion);

	auto bodyRes = body->memcheck(bodyState);

	lemniDestroyMemCheckState(bodyState);

	if(bodyRes.hasError) return bodyRes;

	state->regionMap[this] = fnRegion;

	LemniMemCheckResult res;
	res.hasError = false;
	res.res = { .expr = this, .size = lemniRegionSize(fnRegion, true, 0) };
	return res;
}

LemniMemCheckResult LemniTypedFnDefExprT::memcheck(LemniMemCheckState state) const noexcept{
	return lambda->memcheck(state);
}
