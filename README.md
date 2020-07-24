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
- ArbLib

### Building

Taking for granted a bash-like shell, from the source dir run the following:

```bash
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j4
```

## REPL

A REPL is included to test out the language quickly.

### Building

Follow the instructions under [Library](#Library)

### Usage

Command: `lemni-repl [options]`

Here `options` can be one or more of the following:

| Option | Description |
|:-------|:------------|
| `-i filename` | Evaluate a file before prompting for user input |
| `-e "expr"`   | Evaluate an expression and print the result     |

If run without any options you should get a blank interpreter instance to play around with.

To start the REPL from the build directory, run the following command:

```bash
./repl/lemni-repl
```

You should be met with a prompt like so:

```bash
Infinity lang REPL v4.2 rev 0
Enter replHelp () for help, or replQuit () to quit

>
```

To get a feel for the language try calling `replTut ()`.

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
	if Chars.isSpace (last name) then
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

## Special thanks

Thank you to all the developers behind the following open-source projects that made this possible:

- [ICU4C](http://site.icu-project.org/)
- [GNU MP](https://gmplib.org/)
- [GNU MPFR](https://www.mpfr.org/)
- [ArbLib](http://arblib.org/)
- [UTF8-CPP](https://github.com/nemtrif/utfcpp)
- [{fmt}](https://fmt.dev/)
- [replxx](https://github.com/AmokHuginnsson/replxx)
- [ImTui](https://github.com/ggerganov/imtui)
- [Layout](https://github.com/randrew/layout)
