# The Lemni Programming Language

[![Build Status](https://travis-ci.org/RamblingMadMan/lemni.svg?branch=master)](https://travis-ci.org/RamblingMadMan/lemni)

Lemni is a functional programming language I created for the hell of it. It is designed with the intent of being referentially transparent while still being able to use (some) procedural techniques.

## Library

Included is a library for lexing, parsing, evaluating and compiling lemni source code. It has a C11 API with C++20 wrappers. It is implemented in C++20.

For an example of lexing and parsing take a look at `testall/main.cpp`.

### Dependencies

- ICU4C
- GNU MP
- GNU MPFR

### Building

Taking for granted a bash-like shell, from the source dir run the following:

```bash
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j4
```

## Lemni Example

```
IO    = import "IO"
Chars = import "Chars"

prompt(msg) =
	IO.out msg
	IO.in ()

capitalize(name) =
	(Chars.toUpper (head name)) ++ (tail name)

stripLeadingWs(name) =
	res = name
	
	while Chars.isSpace (head res) do
		res <- tail res
	
	res

stripTrailingWs(name) =
	if Chars.isSpace (last name) do
		stripTrailingWs (init name)
	else
		name

main() =
	name = prompt "What's your name? "
	name = stripTrailingWs name
	name = stripLeadingWs name
	name = capitalize name
	IO.outln ("Hello, " ++ name ++ "!")
``` 
