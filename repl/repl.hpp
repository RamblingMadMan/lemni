#ifndef LEMNI_REPL_REPL_HPP
#define LEMNI_REPL_REPL_HPP 1

#include <vector>
#include <tuple>
#include <string>

#include "fmt/core.h"

#include "lemni/Interop.h"

inline bool showTypes = true;

inline void replHelp(){
	std::vector<std::tuple<std::string, std::string, std::string>> cmds = {
		std::make_tuple("Repl.quit", "Unit -> Bottom", "Shutdown the repl"),
		std::make_tuple("Repl.help", "Unit -> Unit", "Show this help message"),
		std::make_tuple("Repl.showTypes", "Bool -> Unit", "Sets if the repl should print result types"),
		std::make_tuple("Repl.tut", "Unit -> Unit", "Run a small tutorial program")
	};

	std::size_t cmdW = 9;
	std::size_t typeW = 9;
	std::size_t descW = 9;

	for(auto &&cmd : cmds){
		cmdW = std::max(cmdW, std::get<0>(cmd).length() + 2);
		typeW = std::max(typeW, std::get<1>(cmd).length() + 2);
		descW = std::max(descW, std::get<2>(cmd).length() + 1);
	}

	fmt::print(
		"{:>{}} | {:>{}} | {:>{}}\n{:->{}}\n",
		"command", cmdW,
		"type", typeW,
		"description", descW,
		"", descW + cmdW + typeW + 6
	);

	for(auto &&cmd : cmds){
		fmt::print(
			"{:>{}} | {:>{}} | {:>{}}\n",
			std::get<0>(cmd), cmdW,
			std::get<1>(cmd), typeW,
			std::get<2>(cmd), descW
		);
	}

	std::fflush(stdout);
}

inline void replShowTypes(const LemniBool doShow){
	showTypes = !!doShow;
}

inline void replTut(){
	fmt::print(stderr, "Tutorial unimplemented\n");
}

[[noreturn]]
inline void replQuit(){
	fmt::print(stderr, "\n");
	std::exit(EXIT_SUCCESS);
}

#endif // !LEMNI_REPL_REPL_HPP
