# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(overdraw_draw)
oglplus_add_glsl_program(overdraw_screen)

add_executable(app-024_overdraw main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-024_overdraw
	INPUT_TARGETS overdraw_draw overdraw_screen
	PACKED
)
eagine_add_exe_analysis(app-024_overdraw)

add_dependencies(app-024_overdraw eagine-app-examples)

target_link_libraries(
	app-024_overdraw
	PUBLIC EAGine::App
)

set_target_properties(
	app-024_overdraw
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-024_overdraw
	DESTINATION share/eagine/example/app
)