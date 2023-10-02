# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-022_single_pass_edges main.cpp resources.cpp)
add_dependencies(app-022_single_pass_edges eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-022_single_pass_edges
	RESOURCES
		VertShader "vertex.glsl"
		GeomShader "geometry.glsl"
		FragShader "fragment.glsl"
		Program  "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-022_single_pass_edges)

eagine_target_modules(
	app-022_single_pass_edges
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-022_single_pass_edges
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-022_single_pass_edges
	DESTINATION share/eagine/example/app
)
