#include <iostream>
#include <sstream>
#include <vector>

#include "lemni/lex.h"
#include "lemni/parse.h"

constexpr char testStr[] =
R"(f(x) = 2 * x
World
)";

template<typename ... Fns> struct Overloaded: Fns...{ using Fns::operator()...; };
template<typename ... Fns> Overloaded(Fns...) -> Overloaded<Fns...>;

std::vector<lemni::Token> lexAll(std::string_view src){
	auto res = lemni::lexAll(src);
	if(auto err = std::get_if<lemni::LexError>(&res)){
		std::ostringstream ss;
		ss << "Lexing error[" << err->loc.line << '.' << err->loc.col << "]: " << err->msg << '\n';
		throw std::runtime_error(ss.str());
	}
	else{
		return std::get<std::vector<lemni::Token>>(res);
	}
}

bool handleExpr(lemni::Expr expr){
	if(!expr) return true;

	if(auto fnDef = lemni::exprAsFnDef(expr)){
		auto name = lemni::fnDefExprName(fnDef);
		std::cout << "Fn def '" << name << "'\n";
	}

	return false;
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	auto tokens = lexAll(testStr);

	lemni::ParseState state(tokens.data(), tokens.size());

	while(1){
		if(std::visit(
			Overloaded{
				[](const lemni::ParseError &err){
					std::cerr << "Parsing error[" << err.loc.line << '.' << err.loc.col << "]: " << err.msg << '\n';
					return true;
				},
				[](lemni::Expr expr){ return handleExpr(expr); }
			},
			lemni::parse(state)
		))
			break;
	}

	return 0;
}
