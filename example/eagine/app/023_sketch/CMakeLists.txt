# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-023_sketch main.cpp resources.cpp)
add_dependencies(app-023_sketch eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-023_sketch
	RESOURCES
		VertShader "vertex.glsl"
		GeomShader "geometry.glsl"
		FragShader "fragment.glsl"
		Program  "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-023_sketch)

eagine_target_modules(
	app-023_sketch
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-023_sketch
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-023_sketch
	DESTINATION share/eagine/example/app
)
