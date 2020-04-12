set(
	LEMNI_REPL_SOURCES
	main.cpp
)

add_executable(lemni-repl ${LEMNI_REPL_SOURCES})

target_link_libraries(lemni-repl utf8::cpp replxx lemni)
