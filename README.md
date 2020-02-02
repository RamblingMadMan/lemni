# The Lemni Programming Language

Lemni is a functional programming language I created for the hell of it. It is designed with the intent of being referentially transparent while still being able to use (some) procedural techniques.

## Dependencies

- ICU4C
- GNU MP
- GNU MPFR

## Example

```
IO = import "IO"

prompt(msg) =
	IO.out msg
	IO.in

main() =
	name = prompt "What's your name? "
	IO.outln ("Hello, " ++ name ++ "!")
``` 
