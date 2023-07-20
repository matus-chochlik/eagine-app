# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-027_labeled_shapes main.cpp resources.cpp)
add_dependencies(app-027_labeled_shapes eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-027_labeled_shapes
	RESOURCES
		VertShader "vertex.glsl"
		GeomShader "geometry.glsl"
		FragShader "fragment.glsl"
		Program  "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-027_labeled_shapes)

eagine_target_modules(
	app-027_labeled_shapes
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-027_labeled_shapes
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-027_labeled_shapes
	DESTINATION share/eagine/example/app
)
