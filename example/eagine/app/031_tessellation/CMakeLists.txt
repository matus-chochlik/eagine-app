# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(sphere_tessellation)
add_executable(app-031_tessellation main.cpp resources.cpp)
eagine_embed_target_resources(
	TARGET app-031_tessellation
	INPUT_TARGETS sphere_tessellation
	PACKED
)
eagine_add_exe_analysis(app-031_tessellation)

add_dependencies(app-031_tessellation eagine-app-examples)

target_link_libraries(
	app-031_tessellation
	PUBLIC EAGine::App
)

set_target_properties(
	app-031_tessellation
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-031_tessellation
	DESTINATION share/eagine/example/app
)
