find_package(ICU REQUIRED COMPONENTS uc)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

set(
	LEMNI_LIB_SOURCES
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
	Type.cpp
)

add_library(lemni SHARED ${LEMNI_HEADERS} ${LEMNI_LIB_SOURCES})

target_include_directories(lemni PRIVATE ${GMP_INCLUDES} ${MPFR_INCLUDES})

target_link_libraries(lemni PRIVATE ICU::uc utf8::cpp ${GMP_LIBRARIES} ${MPFR_LIBRARIES})
