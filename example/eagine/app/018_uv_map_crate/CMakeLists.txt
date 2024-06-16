# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-018_uv_map_crate main.cpp)
add_dependencies(app-018_uv_map_crate eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-018_uv_map_crate
	RESOURCES
		VertShdr   "vertex.glsl"
		FragShdr   "fragment.glsl"
		Program    "prog.json"
		Crate      "crate.json"
		CrateColor "crate_color.eagitex"
		CrateClImg "crate_color.eagitexi"
		CrateLight "crate_light.eagitex"
		CrateLtImg "crate_light.eagitexi"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-018_uv_map_crate)

eagine_target_modules(
	app-018_uv_map_crate
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-018_uv_map_crate
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-018_uv_map_crate
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)
