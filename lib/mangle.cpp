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

#include <memory>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;

#include "fmt/format.h"

#include "lemni/mangle.h"

struct LemniManglerT{
	std::vector<std::unique_ptr<std::string>> strs;
};

LemniMangler lemniCreateMangler(){
	auto mem = std::malloc(sizeof(LemniManglerT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniManglerT;

	return p;
}

void lemniDestroyMangler(LemniMangler mangler){
	std::destroy_at(mangler);
	std::free(mangler);
}

LemniStr lemniMangler(
	LemniMangler mangler,
	LemniModule module, const LemniStr name,
	LemniType result,
	LemniNat32 numParams, LemniType *const params
){
	auto prefix = "_"s;
	if(module){
		auto moduleName = lemni::toStdStr(lemniModuleId(module));
		prefix = fmt::format("_m{}{}", moduleName.size(), moduleName);
	}

	auto str = fmt::format("{}f{}{}", prefix, numParams, lemni::toStdStrView(lemniTypeMangled(result)));

	for(LemniNat32 i = 0; i < numParams; i++){
		auto param = params[i];
		str += lemni::toStdStrView(lemniTypeMangled(param));
	}

	str += lemni::toStdStrView(name);

	auto strPtr = std::make_unique<std::string>(std::move(str));
	auto ret = lemni::fromStdStrView(*strPtr);

	mangler->strs.emplace_back(std::move(strPtr));

	return ret;
}
