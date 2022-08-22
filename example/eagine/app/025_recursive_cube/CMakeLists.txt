# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(recursive_cube)
add_executable(app-025_recursive_cube main.cpp resources.cpp)
add_dependencies(app-025_recursive_cube eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-025_recursive_cube
	INPUT_TARGETS recursive_cube
)
eagine_add_exe_analysis(app-025_recursive_cube)

if(${WITH_MODULES})
	eagine_target_modules(
		app-025_recursive_cube
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-025_recursive_cube
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-025_recursive_cube
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-025_recursive_cube
	DESTINATION share/eagine/example/app
)
