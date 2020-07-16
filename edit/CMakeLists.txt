find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(Freetype REQUIRED)

set(
	LEMNI_EDIT_SOURCES
#	lay.c
	Widgets.hpp
	Widgets.cpp
	main.cpp
)

configure_file("monoid-font/Monoid-Regular.ttf" "monoid-font/Monoid-Regular.ttf" COPYONLY)
configure_file("monoid-font/Monoid-Bold.ttf" "monoid-font/Monoid-Bold.ttf" COPYONLY)
configure_file("monoid-font/Monoid-Italic.ttf" "monoid-font/Monoid-Italic.ttf" COPYONLY)

add_executable(lemni-edit ${LEMNI_EDIT_SOURCES})

target_include_directories(lemni-edit PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

target_link_libraries(
	lemni-edit
	lemni fmt::fmt
	Freetype::Freetype harfbuzz
	${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES}
)
