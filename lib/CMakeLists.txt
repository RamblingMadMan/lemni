find_package(ICU REQUIRED COMPONENTS uc)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(ARB REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

llvm_map_components_to_libnames(llvm_libs support core irreader)

set(
	LEMNI_LIB_SOURCES
	Interop.cpp
	lex.cpp
	AInt.hpp
	AInt.cpp
	ARatio.hpp
	ARatio.cpp
	AReal.hpp
	AReal.cpp
	Operator.cpp
	Expr.hpp
	Expr.cpp
	parse.cpp
	Type.hpp
	Type.cpp
	Scope.cpp
	TypedExpr.hpp
	TypedExpr.cpp
	typecheck.cpp
	Region.hpp
	Region.cpp
	memcheck.cpp
	Value.hpp
	Value.cpp
	Module.cpp
	TypeList.hpp
	mangle.cpp
	eval.cpp
	GCCJIT.hpp
	LLVM.hpp
	compile.cpp
)

add_library(lemni SHARED ${LEMNI_HEADERS} ${LEMNI_LIB_SOURCES})

target_include_directories(lemni PUBLIC ${LEMNI_INCLUDE_DIR})

target_include_directories(lemni PRIVATE ${GMP_INCLUDES} ${MPFR_INCLUDES} ${ARB_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

target_link_libraries(lemni ICU::uc utf8::cpp fmt::fmt ${GMP_LIBRARIES} ${MPFR_LIBRARIES} ${ARB_LIBRARIES} ffi ${llvm_libs})
