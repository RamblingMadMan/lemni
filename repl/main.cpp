#include <cstdio>
#include <cinttypes>

#include <string_view>
#include <tuple>
using namespace std::literals;

#if __has_include(<ncurses.h>)
#include <ncurses.h>
#endif

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
	for(std::size_t i = 0; i < exprs.size(); i++){
		auto typedExpr = exprs[i];
		auto val = lemni::eval(evalState, typedExpr);
		if(!std::visit(
			Overload{
				[](LemniEvalError err){
					std::fprintf(stderr, "[Eval Error] %.*s\n", int(err.msg.len), err.msg.ptr);
					return false;
				},
				[i, typedExpr, &repl](const lemni::Value &val){
					auto str = val.toString();
					auto type = lemniTypedExprType(typedExpr);
					auto typeStr = lemniTypeStr(type);

					if(showTypes)
						repl.print("%" PRIu64 ": %.*s\n", i, typeStr.len, typeStr.ptr);

					repl.print(" -> %.*s\n", str.size(), str.c_str());

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

struct TestT{
	short x;
	double y;
	short z;
	int w;
};

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	/*
	enum DataType{
		DATA_INT, DATA_FLOAT, DATA_DOUBLE, DATA_SHORT
	};

	constexpr size_t dataTypeAlignments[] = {
		alignof(int), alignof(float), alignof(double), alignof(short)
	};

	constexpr size_t dataTypeSizes[] = {
		sizeof(int), sizeof(float), sizeof(double), sizeof(short)
	};

	constexpr std::string_view dataMemberNames[] = {
		"x", "y", "z", "w"
	};

	constexpr DataType dataMembers[] = {
		DATA_SHORT, DATA_DOUBLE, DATA_SHORT, DATA_INT
	};

	constexpr size_t numDataMembers = sizeof(dataMembers)/sizeof(*dataMembers);

	size_t alignment = 1;
	size_t size = 0;

	fmt::print("offsetof | member\n"
			   "-----------------\n");

	for(size_t i = 0; i < numDataMembers; i++){
		auto t = dataMembers[i];
		auto s = dataTypeSizes[t];
		auto a = dataTypeAlignments[t];
		if(size % a != 0){
			auto offset = a - size % a;
			size += offset;
		}

		alignment = std::max(alignment, a);

		fmt::print("{:>8} |{:>7}\n", size, dataMemberNames[i]);
		size += s;
	}

	fmt::print("calced size:  {}\n", size);
	fmt::print("calced align: {}\n", alignment);
	*/

	replxx::Replxx repl;

	repl.clear_screen();

	fmt::print("Infinity lang REPL v{}.{} rev {}\n", ILANG_REPL_MAJ, ILANG_REPL_MIN, ILANG_REPL_REV);

	fmt::print(
		"Enter {} for help, or {} to quit\n",
		fmt::format(fmt::fg(fmt::color::pink), "Repl.help ()"),
		fmt::format(fmt::fg(fmt::color::pink), "Repl.quit ()")
	);

	repl.set_highlighter_callback(lemniHighlightCb);

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
