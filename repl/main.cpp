#include <cstdio>
#include <cinttypes>

#include "utf8.h"

#include "replxx.hxx"

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/typecheck.h"
#include "lemni/eval.h"

namespace {
	template<typename ... Fns> struct Overload: Fns...{ using Fns::operator()...; };
	template<typename ... Fns> Overload(Fns...) -> Overload<Fns...>;
}

void lemniHighlightCb(std::string const& input, replxx::Replxx::colors_t& colors){
	auto it = cbegin(input);
	auto end = cend(input);
}

template<typename Error>
void errorCallback(const Error &err){
	std::fprintf(stderr, "[Error] %u.%u: %s\n", err.loc.line, err.loc.col, err.msg.ptr);
}

void typedCallback(replxx::Replxx &repl, lemni::EvalState &evalState, LemniTypeSet types, const std::pair<lemni::TypecheckState, std::vector<lemni::TypedExpr>> &p){
	(void)types;
	for(std::size_t i = 0; i < p.second.size(); i++){
		auto val = lemni::eval(evalState, p.second[i]);
		if(!std::visit(
			Overload{
				[&repl](LemniEvalError err){
					repl.print("Eval Error: %.*s\n", err.msg.len, err.msg.ptr);
					return false;
				},
				[i, &repl](const lemni::Value &val){
					auto str = val.toString();
					repl.print("%" PRIu64 " -> %.*s\n", i, str.size(), str.c_str());
					return true;
				}
			},
			val
		 )){
			break;
		}
	}
}

void exprsCallback(replxx::Replxx &repl, lemni::EvalState &evalState, LemniTypeSet types, const std::pair<lemni::ParseState, std::vector<lemni::Expr>> &p){
	auto typecheckRes = lemni::typecheckAll(types, p.second);
	std::visit(
		Overload{
			std::function(errorCallback<lemni::TypecheckError>),
			std::bind(typedCallback, std::ref(repl), std::ref(evalState), types, std::placeholders::_1)
		},
		typecheckRes
	);
}

void tokensCallback(replxx::Replxx &repl, lemni::EvalState &evalState, LemniTypeSet types, const std::vector<lemni::Token> &toks){
	auto parseRes = lemni::parseAll(toks);
	std::visit(
		Overload{
			std::function(errorCallback<lemni::ParseError>),
			std::bind(exprsCallback, std::ref(repl), std::ref(evalState), types, std::placeholders::_1)
		},
		parseRes
	);
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	replxx::Replxx repl;

	repl.set_highlighter_callback(lemniHighlightCb);

	bool running = true;

	lemni::TypeSet types;
	lemni::EvalState evalState;

	while(running){
		std::string_view line = repl.input("> ");
		repl.history_add(std::string(line));

		if(line == "`q"){
			running = false;
		}
		else{
			auto toksRes = lemni::lexAll(line);

			std::visit(
				Overload{
					std::function(errorCallback<lemni::LexError>),
					std::bind(tokensCallback, std::ref(repl), std::ref(evalState), types.handle(), std::placeholders::_1)
				},
				toksRes
			);

			// parse and evaluate
		}
	}

	return 0;
}
