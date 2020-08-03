#include <cstdio>
#include <cinttypes>

#include <vector>
#include <utility>
#include <map>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <tuple>
using namespace std::literals;
namespace fs = std::filesystem;

#include "fmt/core.h"
#include "fmt/color.h"

#include "utf8.h"

#include "replxx.hxx"

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/typecheck.h"
#include "lemni/eval.h"

#include "repl.hpp"

namespace {
	template<typename ... Fns> struct Overload: Fns...{ using Fns::operator()...; };
	template<typename ... Fns> Overload(Fns...) -> Overload<Fns...>;
}

void lemniHighlightCb(std::string const& input, replxx::Replxx::colors_t& colors){
	auto state = lemni::LexState(input);

	std::size_t cpIdx = 0;

	for(;;){
		auto res = lemni::lex(state);
		if(auto err = std::get_if<LemniLexError>(&res)){
			while(cpIdx < colors.size()){
				colors[cpIdx] = replxx::Replxx::Color::BRIGHTRED;
				++cpIdx;
			}

			return;
		}

		auto tok = std::get_if<LemniToken>(&res);
		switch(tok->type){
			case LEMNI_TOKEN_ID:{
				for(std::size_t i = 0; i < tok->text.len; i++){
					colors[cpIdx] = replxx::Replxx::Color::BRIGHTCYAN;
					++cpIdx;
				}
				break;
			}

			case LEMNI_TOKEN_OCTAL:
			case LEMNI_TOKEN_HEX:
			case LEMNI_TOKEN_NAT:
			case LEMNI_TOKEN_BINARY:
			case LEMNI_TOKEN_REAL:{
				for(std::size_t i = 0; i < tok->text.len; i++){
					colors[cpIdx] = replxx::Replxx::Color::BRIGHTMAGENTA;
					++cpIdx;
				}
				break;
			}

			case LEMNI_TOKEN_OP:{
				for(std::size_t i = 0; i < tok->text.len; i++){
					colors[cpIdx] = replxx::Replxx::Color::BRIGHTBLUE;
					++cpIdx;
				}
				break;
			}

			case LEMNI_TOKEN_STR:{
				for(std::size_t i = 0; i < tok->text.len; i++){
					colors[cpIdx] = replxx::Replxx::Color::BRIGHTGREEN;
					++cpIdx;
				}
				break;
			}

			case LEMNI_TOKEN_INDENT:
			case LEMNI_TOKEN_SPACE:{
				cpIdx += tok->text.len;
				break;
			}

			case LEMNI_TOKEN_NEWLINE:{
				++cpIdx;
				break;
			}

			case LEMNI_TOKEN_EOF:{
				return;
			}

			case LEMNI_TOKEN_DEINDENT:
			default:{
				return;
			}
		}
	}
}

template<typename Error>
void errorCallback(const std::string_view errType, const Error &err){
	std::fprintf(stderr, "[%.*s] %u.%u: %.*s\n", int(errType.size()), errType.data(), err.loc.line, err.loc.col, int(err.msg.len), err.msg.ptr);
}

void typedCallback(replxx::Replxx &repl, lemni::EvalState &evalState, lemni::TypecheckState &typeState, const std::vector<lemni::TypedExpr> &exprs){
	(void)typeState;

	std::vector<lemni::Value> stored;
	stored.reserve(exprs.size());

	for(std::size_t i = 0; i < exprs.size(); i++){
		auto typedExpr = exprs[i];
		auto val = lemni::eval(evalState, typedExpr);
		if(!std::visit(
			Overload{
				[](LemniEvalError err){
					std::fprintf(stderr, "[Eval Error] %.*s\n", int(err.msg.len), err.msg.ptr);
					return false;
				},
				[i, typedExpr, &repl, &stored](lemni::Value val){
					auto str = val.toString();
					auto type = lemniTypedExprType(typedExpr);
					auto typeStr = lemniTypeStr(type);

					if(showTypes)
						repl.print("%" PRIu64 ": %.*s\n", i, typeStr.len, typeStr.ptr);

					repl.print(" -> %.*s\n", str.size(), str.c_str());

					stored.emplace_back(std::move(val));

					return true;
				}
			},
			val
		 )){
			break;
		}
	}
}

void exprsCallback(replxx::Replxx &repl, lemni::EvalState &evalState, lemni::TypecheckState &typeState, const std::vector<lemni::Expr> &exprs){
	auto typecheckRes = lemni::typecheckAll(typeState, exprs);
	std::visit(
		Overload{
			std::bind(errorCallback<lemni::TypecheckError>, "Typechecking error"sv, std::placeholders::_1),
			std::bind(typedCallback, std::ref(repl), std::ref(evalState), std::ref(typeState), std::placeholders::_1)
		},
		typecheckRes
	);
}

void tokensCallback(replxx::Replxx &repl, lemni::EvalState &evalState, lemni::TypecheckState &typeState, const std::vector<lemni::Token> &toks){
	auto parseRes = lemni::parseAll(toks);
	std::visit(
		Overload{
			std::bind(errorCallback<lemni::ParseError>, "Parsing error"sv, std::placeholders::_1),
			std::bind(exprsCallback, std::ref(repl), std::ref(evalState), std::ref(typeState), std::placeholders::_1)
		},
		parseRes.second
	);
}

uint16_t toU16(LemniBinaryOp op){
	return static_cast<uint16_t>(op);
}

#define ILANG_REPL_MAJ 0
#define ILANG_REPL_MIN 1
#define ILANG_REPL_REV 0

int main(int argc, char *argv[]){
	std::vector<fs::path> paths;
	std::vector<std::string_view> exprs;

	for(int i = 1; i < argc; i++){
		if(argc % 2 != 1){
			fmt::print(stderr, "Wrong number of parameters.\nUsage: {} [-i filename | -e \"expr\"]\n", argv[0]);
			return -1;
		}

		auto arg = std::string_view(argv[i]);
		if(arg == "-i"){
			auto path = fs::path(argv[++i]);
			if(!fs::exists(path) || !fs::is_regular_file(path)){
				fmt::print(stderr, "File '{}' does not exist or is not a regular file\n", path.c_str());
				return -2;
			}

			paths.emplace_back(path);
		}
		else if(arg == "-e"){
			exprs.emplace_back(argv[++i]);
		}
		else{
			fmt::print(stderr, "Unexpected parameter '{}'\nUsage: {} [-i filename | -e \"expr\"]\n", arg, argv[0]);
			return -1;
		}
	}

	auto types = lemni::TypeSet();
	auto mods = lemni::ModuleMap(types);

	auto typeState = lemni::TypecheckState(mods);
	auto evalState = lemni::EvalState(types);

	auto bottomType = types.bottom();
	auto bottomTypeT = lemniBottomAsType(bottomType);
	auto unitType = types.unit();
	auto unitTypeT = lemniUnitAsType(unitType);
	auto boolType = types.bool_();
	auto boolTypeT = lemniBoolAsType(boolType);
	auto paramName = LEMNICSTR("doShow");

	auto replModule = lemni::Module(mods, LEMNICSTR("Repl"));

	replxx::Replxx repl;

	repl.clear_screen();

	for(auto &&p : paths){
		std::ifstream file(p);
		std::string src, tmp;
		while(std::getline(file, tmp))
			src += tmp + '\n';

		auto lexed = lemni::lexAll(src);
		auto toks = std::visit(
			Overload{
				[&](std::vector<lemni::Token> toks){ return toks; },
				[&](LemniLexError err) -> std::vector<lemni::Token>{
					fmt::print(stderr, "Error lexing file '{}'@{}.{}: {}\n",
						p.c_str(), err.loc.line, err.loc.col, lemni::toStdStrView(err.msg));

					std::exit(-3);
				}
			},
			lexed
		);

		auto parsed = lemni::parseAll(toks);
		auto exprs = std::visit(
			Overload{
				[&](std::vector<lemni::Expr> exprs){ return exprs; },
				[&](LemniParseError err) -> std::vector<lemni::Expr>{
					fmt::print(stderr, "Error parsing file '{}'@{}.{}: {}\n",
						p.c_str(), err.loc.line, err.loc.col, lemni::toStdStrView(err.msg));

					std::exit(-3);
				}
			},
			parsed.second
		);
		
		auto typed = lemni::typecheckAll(typeState, exprs);
		auto typedExprs = std::visit(
			Overload{
				[&](std::vector<lemni::TypedExpr> exprs){ return exprs; },
				[&](LemniTypecheckError err) -> std::vector<lemni::TypedExpr>{
					fmt::print(stderr, "Error typechecking file '{}'@{}.{}: {}\n",
						p.c_str(), err.loc.line, err.loc.col, lemni::toStdStrView(err.msg));

					std::exit(-3);
				}
			},
			typed
		);
	}

	for(auto expr : exprs){
		auto lexed = lemni::lexAll(expr);
		auto toks = std::visit(
			Overload{
				[&](std::vector<lemni::Token> toks){ return toks; },
				[&](LemniLexError err) -> std::vector<lemni::Token>{
					fmt::print(stderr, "Error lexing expression '{}': {}\n",
						expr, lemni::toStdStrView(err.msg));

					std::exit(-4);
				}
			},
			lexed
		);

		auto parsed = lemni::parseAll(toks);
		auto exprs = std::visit(
			Overload{
				[&](auto v){ return v; },
				[&](LemniParseError err) -> std::vector<lemni::Expr>{
					fmt::print(stderr, "Error parsing expression '{}': {}\n",
						expr, lemni::toStdStrView(err.msg));

					std::exit(-4);
				}
			},
			parsed.second
		);

		auto typed = lemni::typecheckAll(typeState, exprs);
		auto typedExprs = std::visit(
			Overload{
				[&](auto v){ return v; },
				[&](LemniTypecheckError err) -> std::vector<lemni::TypedExpr>{
					fmt::print(stderr, "Error parsing expression '{}': {}\n",
						expr, lemni::toStdStrView(err.msg));

					std::exit(-4);
				}
			},
			typed
		);

		for(auto typedExpr : typedExprs){
			auto evaled = lemni::eval(evalState, typedExpr);
			auto value = std::visit(
				Overload{
					[&](auto v){ return v; },
					[&](LemniEvalError err) -> lemni::Value{
						fmt::print(stderr, "Error evaluating expression '{}': {}\n",
							expr, lemni::toStdStrView(err.msg));

						std::exit(-4);
					}
				},
				evaled
			);

			auto str = value.toString();
			fmt::print("{}\n", str);
		}
	}

	if(paths.empty() && exprs.empty()){
		fmt::print("Infinity lang REPL v{}.{} rev {}\n", ILANG_REPL_MAJ, ILANG_REPL_MIN, ILANG_REPL_REV);

		fmt::print(
			"Enter {} for help, or {} to quit\n",
			fmt::format(fmt::fg(fmt::color::pink), "Repl.help ()"),
			fmt::format(fmt::fg(fmt::color::pink), "Repl.quit ()")
		);
	}

	repl.set_highlighter_callback(lemniHighlightCb);

	std::vector<LemniTypedExtFnDeclExpr> extFns = {
		lemniModuleCreateExtFn(
			replModule, LEMNICSTR("help"), reinterpret_cast<void*>(replHelp),
			unitTypeT, 0, nullptr, nullptr
		),
		lemniModuleCreateExtFn(
			replModule, LEMNICSTR("showTypes"), reinterpret_cast<void*>(replShowTypes),
			unitTypeT, 1, &boolTypeT, &paramName
		),
		lemniModuleCreateExtFn(
			replModule, LEMNICSTR("quit"), reinterpret_cast<void*>(replQuit),
			bottomTypeT, 0, nullptr, nullptr
		),
		lemniModuleCreateExtFn(
			replModule, LEMNICSTR("tut"), reinterpret_cast<void*>(replTut),
			unitTypeT, 0, nullptr, nullptr
		)
	};

	mods.register_(replModule);

	auto replModExpr = lemniCreateTypedModule(typeState, LEMNINULLSTR, replModule);
	(void)replModExpr;

	while(true){
		std::string_view line = repl.input("\n> ");
		if(line.empty()){
			continue;
		}

		repl.history_add(std::string(line));

		if(line == ":q"){
			replQuit();
		}
		else{
			auto toksRes = lemni::lexAll(line);

			std::visit(
				Overload{
					std::bind(errorCallback<lemni::LexError>, "Lexing error"sv, std::placeholders::_1),
					std::bind(tokensCallback, std::ref(repl), std::ref(evalState), std::ref(typeState), std::placeholders::_1)
				},
				toksRes
			);
		}
	}

	return 0;
}
