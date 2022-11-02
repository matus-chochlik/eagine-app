# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-013_tiling main.cpp)
add_dependencies(app-013_tiling eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-013_tiling
	RESOURCES
		VertShader "vertex.glsl"
		FragShader "fragment.glsl"
		Program    "program.json"
		TilingTex  "tiling.eagitex"
		TilingImg  "tiling.eagitexi"
		BlocksTex  "blocks.eagitex"
		BlocksImg  "blocks.eagitexi"
		CnnctsTex  "connections.eagitex"
		CnnctsImg  "connections.eagitexi"
		NodesTex   "nodes.eagitex"
		NodesImg   "nodes.eagitexi"
		ThicketTex "thicket.eagitex"
		ThicketImg "thicket.eagitexi"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-013_tiling)

eagine_target_modules(
	app-013_tiling
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-013_tiling
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-013_tiling
	DESTINATION share/eagine/example/app
)
