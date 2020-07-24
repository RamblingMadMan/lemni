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

#include <unordered_map>

#include "lemni/Scope.h"

struct LemniScopeT{
	LemniScopeConst parent;
	std::unordered_map<std::string, LemniTypedLValueExpr> table;
};

LemniScope lemniCreateScope(LemniScopeConst parent){
	auto mem = std::malloc(sizeof(LemniScopeT));
	auto p = new(mem) LemniScopeT;

	p->parent = parent;

	return p;
}

void lemniDestroyScope(LemniScope s){
	std::destroy_at(s);
	std::free(s);
}

LemniTypedLValueExpr lemniScopeFind(LemniScopeConst s, LemniStr name){
	auto res = s->table.find(lemni::toStdStr(name));
	if(res != end(s->table)){
		return res->second;
	}
	else if(s->parent){
		return lemniScopeFind(s->parent, name);
	}
	else{
		return nullptr;
	}
}

bool lemniScopeSet(LemniScope s, LemniTypedLValueExpr expr){
	auto id = lemniTypedLValueExprId(expr);
	if(lemniScopeFind(s, id)){
		return false;
	}

	auto res = s->table.try_emplace(lemni::toStdStr(id), expr);
	return res.second;
}
