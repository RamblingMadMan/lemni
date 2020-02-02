#include <iostream>
#include <vector>

#include "lemni/lex.h"
#include "lemni/parse.h"

constexpr char testStr[] =
R"(add 1 2.0
World
)";

template<typename ... Fns> struct Overloaded: Fns...{ using Fns::operator()...; };
template<typename ... Fns> Overloaded(Fns...) -> Overloaded<Fns...>;

std::vector<lemni::Token> lexAll(std::string_view src){
	lemni::LexState state(src);

	std::vector<lemni::Token> tokens;

	while(1){
		if(std::visit(
			Overloaded{
				[](const lemni::LexError &err){
					std::cerr << "Lexing error: " << err.msg << '\n';
					return true;
				},
				[&](const lemni::Token &tok){
					if(tok.type == LEMNI_TOKEN_EOF)
						return true;
					else{
						tokens.emplace_back(tok);
					}

					return false;
				}
			},
			lemni::lex(state)
		))
			break;
	}

	return tokens;
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
					std::cerr << "Parsing error: " << err.msg << '\n';
					return true;
				},
				[](lemni::Expr expr){
					if(!expr) return true;
					else return false;
				}
			},
			lemni::parse(state)
		))
			break;
	}

	return 0;
}
