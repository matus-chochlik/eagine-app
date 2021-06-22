# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(cel_shading)
add_executable(app-021_cel_shading main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-021_cel_shading
	INPUT_TARGETS cel_shading
	PACKED
)
eagine_add_exe_analysis(app-021_cel_shading)

add_dependencies(app-021_cel_shading eagine-app-examples)

target_link_libraries(
	app-021_cel_shading
	PUBLIC EAGine::App
)

set_target_properties(
	app-021_cel_shading
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-021_cel_shading
	DESTINATION share/eagine/example/app
)
