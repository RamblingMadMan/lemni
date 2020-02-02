#include <iostream>

#include "lemni/lex.h"

constexpr char testStr[] =
R"(f(x) = x * 2

main(args) = f 6
)";

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	auto lexState = lemniCreateLexState(LEMNICSTR(testStr), LemniLocation{.line = 0, .col = 0});

	while(1){
		auto res = lemniLex(lexState);

		if(res.hasError){
			auto &&error = res.error;
			std::cerr << "Lex error: " << std::string_view(error.msg.ptr, error.msg.len) << '\n';
			break;
		}
		else{
			if(res.token.type == LEMNI_TOKEN_EOF){
				std::cout << "Finished lexing\n";
				break;
			}

			switch(res.token.type){
				case LEMNI_TOKEN_ID:{
					std::cout << "Lexed id:   " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_INT:{
					std::cout << "Lexed int:  " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_REAL:{
					std::cout << "Lexed real: " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_OP:{
					std::cout << "Lexed op:   " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_BRACKET_OPEN:{
					std::cout << "Lexed open bracket: " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_BRACKET_CLOSE:{
					std::cout << "Lexed closed bracket: " << res.token.text << '\n';
					break;
				}

				case LEMNI_TOKEN_SPACE:{
					std::cout << "Lexed space: '" << res.token.text << "'\n";
					break;
				}

				case LEMNI_TOKEN_INDENT:{
					std::cout << "Lexed indent: '" << res.token.text << "'\n";
					break;
				}

				case LEMNI_TOKEN_DEINDENT:{
					std::cout << "Lexed deindent\n";
					break;
				}

				case LEMNI_TOKEN_NEWLINE:{
					std::cout << "Lexed newline\n";
					break;
				}

				default:{
					std::cout << "Lexed token\n";
					break;
				}
			}
		}
	}

	lemniDestroyLexState(lexState);

	return 0;
}
