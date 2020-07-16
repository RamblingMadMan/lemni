#include <map>

#include <libgccjit.h>

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/typecheck.h"
#include "lemni/Module.h"

LEMNI_OPAQUE_T_DEF(LemniModule){
	LemniTypeSet types;
	std::vector<LemniTypedExpr> exprs;
};

LEMNI_OPAQUE_T_DEF(LemniRuntime){
	gcc_jit_context *gccCtx;
	std::map<LemniType, gcc_jit_type*> gccTypes;
	std::vector<gcc_jit_rvalue*> gccExprs;
	gcc_jit_result *gccRes;
};

LemniModule lemniCreateModule(LemniTypeSet types){
	auto mem = std::malloc(sizeof(LemniModuleT));
	auto p = new(mem) LemniModuleT;

	p->types = types;

	return p;
}

// TODO: fix states holding error strings dying
LemniModuleResult lemniLoadModule(LemniTypeSet types, LemniStr pathStr){
	auto mod = lemniCreateModule(types);

	LemniModuleResult res;

	auto lexState = lemni::LexState(pathStr);

	std::vector<LemniToken> toks;

	while(lemniLexStateRemainder(lexState).len > 0){
		auto lexRes = lemniLex(lexState);
		if(lexRes.hasError){
			res.resType = LEMNI_MODULE_RESULT_LEX;
			res.lexErr = lexRes.error;
			return res;
		}

		toks.emplace_back(lexRes.token);
	}

	auto parseState = lemni::ParseState(toks.data(), toks.size());

	std::vector<LemniExpr> exprs;

	while(parseState.numTokens() > 0){
		auto parseRes = lemniParse(parseState);
		if(parseRes.hasError){
			res.resType = LEMNI_MODULE_RESULT_PARSE;
			res.parseErr = parseRes.error;
			return res;
		}

		exprs.emplace_back(parseRes.expr);
	}

	LemniModuleTypecheckCBs cbs;

	cbs.ok = [](void *data, LemniTypedExpr *const, const size_t){
		auto res = reinterpret_cast<LemniModuleResult*>(data);
		res->resType = LEMNI_MODULE_RESULT_MODULE;
	};

	cbs.err = [](void *data, LemniTypecheckError err){
		auto res = reinterpret_cast<LemniModuleResult*>(data);
		res->resType = LEMNI_MODULE_RESULT_TYPECHECK;
		res->typeErr = err;
	};

	cbs.data = &res;

	lemniModuleTypecheck(mod, exprs.data(), exprs.size(), cbs);

	if(res.resType == LEMNI_MODULE_RESULT_MODULE){
		res.module = mod;
	}

	return res;
}

void lemniDestroyModule(LemniModule module){
	std::destroy_at(module);
	std::free(module);
}

LemniTypeSet lemniModuleTypeSet(LemniModule mod){ return mod->types; }

size_t lemniModuleNumExprs(LemniModule mod){ return mod->exprs.size(); }

LemniTypedExpr *lemniModuleExprs(LemniModule mod){ return mod->exprs.data(); }

void lemniModuleTypecheck(LemniModule mod, const LemniExpr *const exprs, const size_t numExprs, LemniModuleTypecheckCBs cbs){
	auto state = lemni::TypecheckState(mod->types);

	std::vector<LemniTypedExpr> typedExprs;
	typedExprs.reserve(numExprs);

	for(size_t i = 0; i < numExprs; i++){
		auto expr = exprs[i];
		auto res = lemniTypecheck(state, expr);
		if(res.hasError){
			cbs.err(cbs.data, res.error);
			return;
		}
		else{
			typedExprs.emplace_back(res.expr);
		}
	}

	mod->exprs.insert(mod->exprs.end(), std::make_move_iterator(begin(typedExprs)), std::make_move_iterator(end(typedExprs)));

	cbs.ok(cbs.data, typedExprs.data(), typedExprs.size());
}

typedef union LemniRegisterT{
	uint8_t bytes[32];
	uint16_t hwords[16];
	uint32_t words[8];
	uint64_t dwords[4];
} LemniRegister;

struct LemniInstrT{
	virtual std::string x64AsmStr(){
		return "noop";
	}
};

LEMNI_OPAQUE_T(LemniInstr);

struct LemniVMStateT{
	LemniRegister registers[32];
};

LEMNI_OPAQUE_T(LemniVMState);

LemniVMState lemniCreateVm(LemniInstrConst *const instrs, const size_t n){
	auto mem = std::malloc(sizeof(LemniVMStateT));
	auto p = new(mem) LemniVMStateT;

	for(size_t i = 0; i < n; i++){
		auto instr = instrs[n];
	}

	return p;
}

struct LemniAddInstrT: LemniInstrT{
	//std::string x64AsmStr() override{ return fmt::format("add {} {}"); }

};

struct LemniSubInstrT: LemniInstrT{

};

struct LemniMulInstrT: LemniInstrT{

};

LemniRuntime lemniModuleJITCompile(LemniModule module){
	auto mem = std::malloc(sizeof(LemniRuntimeT));
	auto p = new(mem) LemniRuntimeT;

	p->gccCtx = gcc_jit_context_acquire();

	for(auto expr : module->exprs){

	}
	// TODO: Compile module

	return p;
}

void lemniDestroyRuntime(LemniRuntime rt){
	gcc_jit_context_release(rt->gccCtx);

	std::destroy_at(rt);
	std::free(rt);
}

/*
LemniBoundParams lemniBindParams(size_t count, ...){
	va_list va;
	va_start(va, count);

	for(size_t i = 0; i < count; i++){
		auto type = va_arg(va, LemniType);
		if(lemniTypeAsProduct(type) || lemniTypeAsRecord(type)){
			auto val = va_arg(va, const void*);
		}
		else if(auto int_ = lemniTypeAsInt(type)){

		}
	}
}
*/

static LemniBoundParams defaultBindParams(void *env, ...){
	(void)env;
	return nullptr;
}

LemniRtFn lemniRuntimeFn(LemniRuntimeConst rt, LemniStr name){
	LemniRtFn ret{
		.createEnv = [](LemniRuntimeConst) -> void*{ return nullptr; },
		.destroyEnv = [](void*){ },
		.bindParams = defaultBindParams,
		.fn = [](void *env, LemniRtValue res, LemniBoundParams args){
			res->type = nullptr;
			res->data = 0;
			(void)env; (void)args;
		}
	};

	return ret;
}
