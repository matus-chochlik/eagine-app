# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(halo_surface)
oglplus_add_glsl_program(halo_halo)

add_executable(app-026_halo main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-026_halo
	INPUT_TARGETS halo_surface halo_halo
	PACKED
)
eagine_add_exe_analysis(app-026_halo)

add_dependencies(app-026_halo eagine-app-examples)

target_link_libraries(
	app-026_halo
	PUBLIC EAGine::App
)

set_target_properties(
	app-026_halo
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-026_halo
	DESTINATION share/eagine/example/app
)
