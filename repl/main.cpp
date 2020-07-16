#include <cstdio>
#include <cinttypes>

#include <string_view>
#include <tuple>
using namespace std::literals;

#include "fmt/core.h"
#include "fmt/color.h"

#include "utf8.h"

#include "replxx.hxx"

#include "imtui/imtui.h"

#if __has_include(<ncurses.h>)
#include "imtui/imtui-impl-ncurses.h"
#endif

#include "lemni/lex.h"
#include "lemni/parse.h"
#include "lemni/typecheck.h"
#include "lemni/eval.h"

bool running = true;
bool showTypes = true;

namespace {
	template<typename ... Fns> struct Overload: Fns...{ using Fns::operator()...; };
	template<typename ... Fns> Overload(Fns...) -> Overload<Fns...>;
}

void lemniHighlightCb(std::string const& input, replxx::Replxx::colors_t& colors){
	auto it = cbegin(input);
	auto end = cend(input);
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

void replHelp(){
	std::vector<std::tuple<std::string, std::string, std::string>> cmds = {
		std::make_tuple("replQuit", "Unit -> Unit", "Shutdown the repl"),
		std::make_tuple("replHelp", "Unit -> Unit", "Show this help message"),
		std::make_tuple("replShowTypes", "Bool -> Unit", "Sets if the repl should print result types"),
		std::make_tuple("replTut", "Unit -> Unit", "Run a small tutorial program")
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

void replShowTypes(const std::uint8_t doShow){
	showTypes = doShow;
}

void replTut(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

#if __has_include(<ncurses.h>)
	auto screen = ImTui_ImplNcurses_Init(true);
	ImTui_ImplText_Init();
#else
	auto screen = ImTui_ImplText_Init();
#endif

	static const auto colorGaqu = ImVec4(0.f, 1.f, 128.f/255.f, 1.f);
	static const auto colorPink = ImVec4(1.f, 192.f/255.f, 203.f/255.f, 1.f);

	bool running = true;

	static char inputBuf[256] = { 0 };
	static std::string inputStr;

	std::uint32_t curStep = 0;

	const char *stepNames[] = { "Operators", "Variables", "Functions", "Quit" };

	auto nextStep = [&]{
		std::memset(inputBuf, 0, sizeof(inputBuf));
		inputStr = "";
		++curStep;
	};

	auto step0 = [&]{
		ImGui::Text("Say we have 2 variables, ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "a = 1");
		ImGui::SameLine(); ImGui::Text(" and ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "b = 2");

		ImGui::NewLine();

		ImGui::Text("Enter an expression to add them together:");

		if(ImGui::InputText("", inputBuf, sizeof(inputBuf)/sizeof(*inputBuf))){
			inputStr = inputBuf;

			inputStr.erase(std::remove_if(inputStr.begin(), inputStr.end(), isspace), inputStr.end());
		}

		if(inputStr == "a+b") nextStep();
	};

	auto step1 = [&]{
		ImGui::Text("Now enter an expression to declare a new variable ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "c");
		ImGui::SameLine(); ImGui::Text(" with the value 3:");

		if(ImGui::InputText("", inputBuf, sizeof(inputBuf)/sizeof(*inputBuf))){
			inputStr = inputBuf;

			inputStr.erase(std::remove_if(inputStr.begin(), inputStr.end(), isspace), inputStr.end());
		}

		if(inputStr == "c=3") nextStep();
	};

	auto step2 = [&]{
		ImGui::Text("Here is a function that adds 2 values together:");
		ImGui::NewLine();
		ImGui::TextColored(colorGaqu, "add");
		ImGui::SameLine(); ImGui::Text("(");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "a");
		ImGui::SameLine(); ImGui::Text(", ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "b");
		ImGui::SameLine(); ImGui::Text(") = ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "a");
		ImGui::SameLine(); ImGui::Text(" + ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "b");
		ImGui::NewLine();
		ImGui::Text("Enter an expression for a function 'sub' that subtracts two parameters ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "a");
		ImGui::SameLine(); ImGui::Text(" and ");
		ImGui::SameLine(); ImGui::TextColored(colorPink, "b");
		ImGui::SameLine(); ImGui::Text(":");

		if(ImGui::InputText("", inputBuf, sizeof(inputBuf)/sizeof(*inputBuf))){
			inputStr = inputBuf;

			inputStr.erase(std::remove_if(inputStr.begin(), inputStr.end(), isspace), inputStr.end());
		}

		if(inputStr == "sub(a,b)=a-b") nextStep();
	};

	auto stepQuit = [&]{ running = false; };

	using step_t = std::function<void()>;

	step_t steps[] = { step0, step1, step2, stepQuit };

	const std::size_t numSteps = sizeof(steps)/sizeof(*steps);

	while(running){
#if __has_include(<ncurses.h>)
		ImTui_ImplNcurses_NewFrame();
#endif
		ImTui_ImplText_NewFrame();

		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(4, 2), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);

		ImGui::Begin(stepNames[curStep]);

		if(ImGui::BeginMainMenuBar()){
			if(ImGui::BeginMenu("Sections")){
				for(std::uint32_t i = 0; i < numSteps; i++){
					if(ImGui::MenuItem(stepNames[i]))
						curStep = i;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		steps[curStep]();

		ImGui::End();

		ImGui::Render();

		ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
		ImTui_ImplNcurses_DrawScreen();
	}

	ImTui_ImplText_Shutdown();

#if __has_include(<ncurses.h>)
	ImTui_ImplNcurses_Shutdown();
#endif
}

[[noreturn]]
void replQuit(){
	fmt::print(stderr, "\n");
	std::exit(EXIT_SUCCESS);
}

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
		fmt::format(fmt::fg(fmt::color::pink), "replHelp ()"),
		fmt::format(fmt::fg(fmt::color::pink), "replQuit ()")
	);

	repl.set_highlighter_callback(lemniHighlightCb);

	auto types = lemni::TypeSet();
	auto typeState = lemni::TypecheckState(types);
	auto evalState = lemni::EvalState(types);

	auto unitType = types.unit();
	auto unitTypeT = lemniUnitAsType(unitType);
	auto boolType = types.bool_();
	auto boolTypeT = lemniBoolAsType(boolType);
	auto paramName = LEMNICSTR("doShow");

	std::vector<LemniTypedExtFnDeclExpr> extFns = {
		lemniCreateTypedExtFn(
			typeState, LEMNICSTR("replHelp"), reinterpret_cast<void*>(replHelp),
			unitTypeT, 0, nullptr, nullptr
		),
		lemniCreateTypedExtFn(
			typeState, LEMNICSTR("replShowTypes"), reinterpret_cast<void*>(replShowTypes),
			unitTypeT, 1, &boolTypeT, &paramName
		),
		lemniCreateTypedExtFn(
			typeState, LEMNICSTR("replQuit"), reinterpret_cast<void*>(replQuit),
			unitTypeT, 0, nullptr, nullptr
		),
		lemniCreateTypedExtFn(
			typeState, LEMNICSTR("replTut"), reinterpret_cast<void*>(replTut),
			unitTypeT, 0, nullptr, nullptr
		)
	};

	while(running){
		std::string_view line = repl.input("\n> ");
		if(line.empty()){
			continue;
		}

		repl.history_add(std::string(line));

		if(line == "`q"){
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
