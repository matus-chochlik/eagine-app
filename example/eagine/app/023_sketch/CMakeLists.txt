# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(sketch)

add_executable(app-023_sketch main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-023_sketch
	INPUT_TARGETS sketch
)
eagine_add_exe_analysis(app-023_sketch)

add_dependencies(app-023_sketch eagine-app-examples)

target_link_libraries(
	app-023_sketch
	PUBLIC EAGine::App
)

set_target_properties(
	app-023_sketch
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-023_sketch
	DESTINATION share/eagine/example/app
)
