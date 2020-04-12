#include <iostream>
#include <sstream>
#include <vector>

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/Value.h"

constexpr char testStr[] =
R"(f(x) = (2 * x)
(1, 2, 3, 4)
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

	if(auto lvalue = lemni::exprAsLValue(expr)){
		if(auto fnDef = lemni::lvalueExprAsFnDef(lvalue)){
			auto name = lemni::fnDefExprName(fnDef);
			std::cout << "Fn def '" << name << "'\n";
		}
	}

	return false;
}

void testValues(){
	using lemni::interop::unit;
	using lemni::Value;

	Value a(6);
	Value b(3);
	Value c = a / b;

	std::cout << a.toString() << " / " << b.toString() << " == " << c.toString() << '\n';

	a = Value(unit);
	b = Value(true);

	try{
		//c = a + b;
	}
	catch(const std::runtime_error &e){
		// good
		std::cerr << e.what() << '\n';
	}
	catch(...){
		throw;
	}

	std::cout << a.toString() << " + " << b.toString() << " == UNDEFINED\n";
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	testValues();

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
