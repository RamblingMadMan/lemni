set(
	LEMNI_REPL_SOURCES
	repl.hpp
	main.cpp
)

add_executable(lemni-repl ${LEMNI_REPL_SOURCES})

set_target_properties(lemni-repl PROPERTIES ENABLE_EXPORTS TRUE)

target_link_libraries(lemni-repl utf8::cpp fmt::fmt replxx lemni)
