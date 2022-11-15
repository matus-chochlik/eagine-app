# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-020_bezier_patch main.cpp resources.cpp)
add_dependencies(app-020_bezier_patch eagine-app-examples)
eagine_embed_target_resources(
	TARGET app-020_bezier_patch
	RESOURCES
		VertShader "vertex.glsl"
		TCtlShader "tes_ctrl.glsl"
		TEvlShader "tes_eval.glsl"
		FragShader "fragment.glsl"
		Program    "prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-020_bezier_patch)

eagine_target_modules(
	app-020_bezier_patch
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-020_bezier_patch
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-020_bezier_patch
	DESTINATION share/eagine/example/app
)
