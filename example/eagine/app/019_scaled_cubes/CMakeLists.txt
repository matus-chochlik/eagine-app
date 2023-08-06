# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-019_scaled_cubes main.cpp resources.cpp)
add_dependencies(app-019_scaled_cubes eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-019_scaled_cubes
	RESOURCES
		VertShader "vertex.glsl"
		FragShader "fragment.glsl"
		Program  "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-019_scaled_cubes)

eagine_target_modules(
	app-019_scaled_cubes
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-019_scaled_cubes
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-019_scaled_cubes
	DESTINATION share/eagine/example/app
)
