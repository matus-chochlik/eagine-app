# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(single_pass_edges)
add_executable(app-022_single_pass_edges main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-022_single_pass_edges
	INPUT_TARGETS single_pass_edges
	PACKED
)
eagine_add_exe_analysis(app-022_single_pass_edges)

add_dependencies(app-022_single_pass_edges eagine-app-examples)

target_link_libraries(
	app-022_single_pass_edges
	PUBLIC EAGine::App
)

set_target_properties(
	app-022_single_pass_edges
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-022_single_pass_edges
	DESTINATION share/eagine/example/app
)
