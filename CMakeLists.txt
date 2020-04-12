cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR) 

project(
	lemni-lang
	DESCRIPTION "The Lemni programming language"
	VERSION 0.1.0
	LANGUAGES CXX C
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options("-fconcepts")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options("-fconcepts")
endif()

set(LEMNI_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(
	LEMNI_HEADERS
	${LEMNI_INCLUDE_DIR}/lemni/Str.h
	${LEMNI_INCLUDE_DIR}/lemni/Location.h
	${LEMNI_INCLUDE_DIR}/lemni/Token.h
	${LEMNI_INCLUDE_DIR}/lemni/lex.h
	${LEMNI_INCLUDE_DIR}/lemni/Operator.h
	${LEMNI_INCLUDE_DIR}/lemni/AInt.h
	${LEMNI_INCLUDE_DIR}/lemni/AReal.h
	${LEMNI_INCLUDE_DIR}/lemni/ARatio.h
	${LEMNI_INCLUDE_DIR}/lemni/Expr.h
	${LEMNI_INCLUDE_DIR}/lemni/parse.h
	${LEMNI_INCLUDE_DIR}/lemni/Interop.h
	${LEMNI_INCLUDE_DIR}/lemni/Type.h
	${LEMNI_INCLUDE_DIR}/lemni/TypedExpr.h
	${LEMNI_INCLUDE_DIR}/lemni/typecheck.h
	${LEMNI_INCLUDE_DIR}/lemni/Value.h
	${LEMNI_INCLUDE_DIR}/lemni/eval.h
)

include_directories(include)

set(UTF8_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8_INSTALL OFF CACHE BOOL "" FORCE)
set(UTF8_SAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/utfcpp)
add_subdirectory(deps/replxx)

add_subdirectory(lib)
add_subdirectory(repl)
add_subdirectory(testall)
