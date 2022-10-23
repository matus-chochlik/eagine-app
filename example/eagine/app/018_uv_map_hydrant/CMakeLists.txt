# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-018_uv_map_hydrant main.cpp)
add_dependencies(app-018_uv_map_hydrant eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-018_uv_map_hydrant
	RESOURCES
		VertShdr "vertex.glsl"
		FragShdr "fragment.glsl"
		HydrntProg "prog.json"
		HydrntShpe "hydrant.json"
		HydrntColr "hydrant_color.eagitexi"
		HydrntNorm "hydrant_normal.eagitexi"
		HydrntLght "hydrant_light.eagitexi"
		HydrantTex "hydrant.eagitex"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-018_uv_map_hydrant)

eagine_target_modules(
	app-018_uv_map_hydrant
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-018_uv_map_hydrant
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-018_uv_map_hydrant
	DESTINATION share/eagine/example/app
)
