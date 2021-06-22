# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(metaballs_metaball)
oglplus_add_glsl_program(metaballs_field)
oglplus_add_glsl_program(metaballs_surface)

add_executable(app-040_metaballs main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-040_metaballs
	INPUT_TARGETS metaballs_metaball metaballs_field metaballs_surface
	PACKED
)
eagine_add_exe_analysis(app-040_metaballs)

add_dependencies(app-040_metaballs eagine-app-examples)

target_link_libraries(
	app-040_metaballs
	PUBLIC EAGine::App
)

set_target_properties(
	app-040_metaballs
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-040_metaballs
	DESTINATION share/eagine/example/app
)
