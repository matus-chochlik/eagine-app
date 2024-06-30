# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-018_uv_map_barrel main.cpp)
add_dependencies(app-018_uv_map_barrel eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-018_uv_map_barrel
	RESOURCES
		VertShdr   "vertex.glsl"
		FragShdr   "fragment.glsl"
		Program    "prog.json"
		Barrel     "barrel.eagimesh"
		BarelClImg "barrel_color.eagitexi"
		BarelColor "barrel_color.eagitex"
		BarelAOImg "barrel_aoccl.eagitexi"
		BarelAOccl "barrel_aoccl.eagitex"
		BarelRgImg "barrel_rough.eagitexi"
		BarelRough "barrel_rough.eagitex"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-018_uv_map_barrel)

eagine_target_modules(
	app-018_uv_map_barrel
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-018_uv_map_barrel
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-018_uv_map_barrel
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)
