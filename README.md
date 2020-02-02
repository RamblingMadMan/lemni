# The Lemni Programming Language

Lemni is a functional programming language I created for the hell of it. It is designed with the intent of being referentially transparent while still being able to use (some) procedural techniques.

## Library

The library for lexing, parsing, evaluating and compiling lemni source code has a C API with C++ wrappers and is implemented in C++.

An example of lexing and parsing:

```
#include <vector>

#include "lemni/lex.h"
#include "lemni/parse.h"

template<typename ... Fns> struct Overloaded: Fns...{ using Fns::operator()...; };
template<typename ... Fns> Overloaded(Fns...) -> Overloaded<Fns...>;

std::vector<lemni::Token> lexAll(std::string_view src){
	lemni::LexState state(src);
	
	std::vector<lemni::Token> tokens;
	
	while(1){=
		bool doBreak = false;
		
		if(std::visit(
			Overloaded{
				[](const lemni::LexError &err){
					/* error */
					return true;
				},
				[&](const lemni::Token &tok){
					/* lexed a token */
					if(tok.type == LEMNI_TOKEN_EOF)
						return true;
					else
						tokens.emplace_back(tok);
					
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
	const char src[] = "hello world";

	auto tokens = lexAll(src);
	
	lemni::ParseState state(tokens.data(), tokens.size());
	
	while(1){
		if(std::visit(
			Overloaded{
				[](const lemni::ParseError &err){
					/* error */
					return true;
				},
				[](lemni::Expr expr){
					/* parsed an expression */
					if(!expr){
						// null expr signifies end of tokens
						return true;
					}
					else{
						// do something with the expression
					}
					
					return false;
				}
			},
			lemni::parse(state)
		))
			break;
	}

	return 0;
}
```

## Dependencies

- ICU4C
- GNU MP
- GNU MPFR

## Lemni Example

```
IO   = import "IO"
Chars = import "Chars"

prompt(msg) =
	IO.out msg
	IO.in

capitalize(name) =
	(Chars.toUpper (head name)) ++ (tail name)

stripLeadingWs(name) =
	res = name
	
	while Chars.isSpace (head res) do
		res <- tail res
	
	res

main() =
	name = prompt "What's your name? "
	name = stripLeadingWs name
	name = capitalize name
	IO.outln ("Hello, " ++ name ++ "!")
``` 
