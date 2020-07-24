#include <fstream>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/typecheck.h"
#include "lemni/compile.h"
#include "lemni/Module.h"

#include "TypedExpr.hpp"

LEMNI_OPAQUE_T_DEF(LemniModule){
	std::string id;
	LemniTypecheckState state;
	std::vector<LemniTypedExpr> exprs;
	std::vector<std::unique_ptr<std::string>> errMsgs;
};

LEMNI_OPAQUE_T_DEF(LemniModuleMap){
	LemniTypeSet types;
	std::vector<lemni::Module> loaded;
	std::map<std::string, std::string, std::less<>> aliased;
	std::map<std::string, LemniModule, std::less<>> mapped;
	std::map<std::string, LemniModule, std::less<>> registered;
};

LEMNI_OPAQUE_T_DEF(LemniRuntime){
	gcc_jit_context *gccCtx;
	std::map<LemniType, gcc_jit_type*> gccTypes;
	std::vector<gcc_jit_rvalue*> gccExprs;
	gcc_jit_result *gccRes;
};

LemniModule lemniCreateModule(LemniModuleMap mods, const LemniStr id){
	if((id.len == 0) || !id.ptr) return nullptr;

	auto mem = std::malloc(sizeof(LemniModuleT));
	auto p = new(mem) LemniModuleT;

	p->id = lemni::toStdStr(id);
	p->state = lemniCreateTypecheckState(mods);

	return p;
}

void lemniDestroyModule(LemniModule module){
	lemniDestroyTypecheckState(module->state);
	std::destroy_at(module);
	std::free(module);
}

LemniStr lemniModuleId(LemniModuleConst mod){
	return lemni::fromStdStrView(mod->id);
}

LemniTypeSet lemniModuleTypeSet(LemniModuleConst mod){
	return lemniModuleMapTypes(lemniTypecheckStateModuleMap(mod->state));
}

LemniTypecheckStateConst lemniModuleTypecheckState(LemniModuleConst mod){
	return mod->state;
}

size_t lemniModuleNumExprs(LemniModule mod){ return mod->exprs.size(); }

LemniTypedExpr *lemniModuleExprs(LemniModule mod){ return mod->exprs.data(); }

LemniTypecheckResult lemniModuleTypecheck(LemniModule mod, LemniExpr expr){
	auto res = lemniTypecheck(mod->state, expr);
	if(res.hasError) return res;

	mod->exprs.emplace_back(res.expr);

	return res;
}

LemniTypedExtFnDeclExpr lemniModuleCreateExtFn(
	LemniModule mod, const LemniStr name, void *const ptr,
	const LemniType resultType,
	const LemniNat64 numParams,
	const LemniType *const paramTypes,
	const LemniStr *const paramNames
){
	auto res = lemniCreateTypedExtFn(mod->state, name, ptr, resultType, numParams, paramTypes, paramNames);
	return res;
}

LemniCompileResult lemniModuleJITCompile(LemniModule module){
	auto state = lemniCreateCompileState(nullptr);

	auto res = lemniCompile(state, module->exprs.data(), module->exprs.size());
	if(res.hasError){
		auto errMsg = std::make_unique<std::string>(lemni::toStdStr(res.error.msg));
		auto &&msg = module->errMsgs.emplace_back(std::move(errMsg));
		res.error.msg = lemni::fromStdStrView(*msg);
		lemniDestroyCompileState(state);
		return res;
	}

	lemniDestroyCompileState(state);

	return res;
}

LemniModuleMap lemniCreateModuleMap(LemniTypeSet types){
	auto mem = std::malloc(sizeof(LemniModuleMapT));
	if(!mem) return nullptr;

	auto p = new(mem) LemniModuleMapT;

	p->types = types;

	return p;
}

void lemniDestroyModuleMap(LemniModuleMap mods){
	std::destroy_at(mods);
	std::free(mods);
}

LemniTypeSet lemniModuleMapTypes(LemniModuleMap mods){
	return mods->types;
}

// TODO: fix states holding error strings dying
LemniModuleResult lemniLoadModule(LemniModuleMap mods, const LemniStr id){
	LemniModuleResult res;

	auto name = id;

	auto pathRes = mods->aliased.find(lemni::toStdStrView(name));
	if(pathRes != end(mods->aliased)){
		name = lemni::fromStdStrView(pathRes->second.c_str());
	}

	auto regRes = mods->registered.find(lemni::toStdStrView(id));
	if(regRes == end(mods->registered)){
		regRes = mods->registered.find(lemni::toStdStrView(name));
	}

	if(regRes != end(mods->registered)){
		res.resType = LEMNI_MODULE_RESULT_MODULE;
		res.module = regRes->second;
		return res;
	}

	auto path = fs::path(lemni::toStdStrView(name));

	auto nameStr = std::string(path.parent_path() / path.stem());

	auto modRes = mods->mapped.find(nameStr);
	if(modRes != end(mods->mapped)){
		res.resType = LEMNI_MODULE_RESULT_MODULE;
		res.module = modRes->second;
		return res;
	}

	if(!fs::exists(path) || !fs::is_regular_file(path)){
		res.resType = LEMNI_MODULE_LEX_ERROR;
		res.lexErr.loc = LemniLocation{ UINT32_MAX, UINT32_MAX };
		res.lexErr.msg = LEMNICSTR("file does not exist");
		return res;
	}

	auto file = std::ifstream(path);

	std::string tmp, src;
	while(std::getline(file, tmp)){
		src += tmp + '\n';
	}

	auto lexState = lemni::LexState(lemni::fromStdStrView(src));

	std::vector<LemniToken> toks;

	while(lemniLexStateRemainder(lexState).len > 0){
		auto lexRes = lemniLex(lexState);
		if(lexRes.hasError){
			res.resType = LEMNI_MODULE_LEX_ERROR;
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
			res.resType = LEMNI_MODULE_PARSE_ERROR;
			res.parseErr = parseRes.error;
			return res;
		}

		exprs.emplace_back(parseRes.expr);
	}

	auto mod = lemniCreateModule(mods, id);

	for(auto expr : exprs){
		auto typeRes = lemniModuleTypecheck(mod, expr);
		if(typeRes.hasError){
			res.resType = LEMNI_MODULE_TYPECHECK_ERROR;
			res.typeErr = typeRes.error;
			return res;
		}
	}

	mods->loaded.emplace_back(lemni::Module::from(mod));
	mods->mapped[nameStr] = mod;

	res.resType = LEMNI_MODULE_RESULT_MODULE;
	res.module = mod;

	return res;
}

void lemniAliasModule(LemniModuleMap mods, const LemniStr id, const LemniStr alias){
	mods->aliased[lemni::toStdStr(alias)] = lemni::toStdStr(id);
}

void lemniRegisterModule(LemniModuleMap mods, LemniModule module){
	mods->registered[module->id] = module;
}
