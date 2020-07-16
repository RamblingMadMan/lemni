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

#include <map>

#include <libgccjit.h>

#include "lemni/compile.h"

#include "TypedExpr.hpp"

struct LemniJitExprT{
	gcc_jit_rvalue *handle;
};

struct LemniCompileStateT{
	gcc_jit_context *ctx;
	std::map<LemniType, gcc_jit_type*> jitTypes;
	std::map<LemniTypedFnDefExpr, gcc_jit_function*> jitFns;
};

struct LemniObjectT{
	gcc_jit_result *res;
};

void lemniDestroyObject(LemniObject obj){
	gcc_jit_result_release(obj->res);

	std::destroy_at(obj);
	std::free(obj);
}

LemniCompileState lemniCreateCompileState(){
	auto mem = std::malloc(sizeof(LemniCompileStateT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniCompileStateT;

	p->ctx = gcc_jit_context_acquire();

	return p;
}

void lemniDestroyCompileState(LemniCompileState state){
	gcc_jit_context_release(state->ctx);

	std::destroy_at(state);
	std::free(state);
}

LemniCompileResult lemniCompile(LemniCompileState state, LemniTypedExpr *const exprs, const size_t numExprs){
	LemniCompileResult res;

	for(std::size_t i = 0; i < numExprs; i++){
		auto expr = exprs[i];
		auto jitRes = expr->jit(state);
		if(jitRes.hasError){
			res.hasError = true;
			res.error.msg = jitRes.error.msg;
			return res;
		}
	}

	res.hasError = true;
	res.error.msg = LEMNICSTR("compilation unimplemented");
	return res;
}
