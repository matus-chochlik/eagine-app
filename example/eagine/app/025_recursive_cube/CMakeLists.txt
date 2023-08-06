# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-025_recursive_cube main.cpp resources.cpp)
add_dependencies(app-025_recursive_cube eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-025_recursive_cube
	RESOURCES
		VertShader "vertex.glsl"
		FragShader "fragment.glsl"
		Program  "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-025_recursive_cube)

eagine_target_modules(
	app-025_recursive_cube
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-025_recursive_cube
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-025_recursive_cube
	DESTINATION share/eagine/example/app
)
