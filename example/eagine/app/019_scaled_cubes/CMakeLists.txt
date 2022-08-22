# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(scaled_cubes)
add_executable(app-019_scaled_cubes main.cpp resources.cpp)
add_dependencies(app-019_scaled_cubes eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-019_scaled_cubes
	INPUT_TARGETS scaled_cubes
	PACKED
)
eagine_add_exe_analysis(app-019_scaled_cubes)

if(${WITH_MODULES})
	eagine_target_modules(
		app-019_scaled_cubes
		eagine.core
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-019_scaled_cubes
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-019_scaled_cubes
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-019_scaled_cubes
	DESTINATION share/eagine/example/app
)
