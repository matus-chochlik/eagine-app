# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/crate_2.json"
	"${CMAKE_CURRENT_BINARY_DIR}/crate.json"
)

add_executable(app-018_uv_map_crate main.cpp)
add_dependencies(app-018_uv_map_crate eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-018_uv_map_crate
	INPUT_TARGETS
		crate_2_color
		crate_2_light
	RESOURCES
		VertShdr "vertex.glsl"
		FragShdr "fragment.glsl"
		Program  "prog.json"
		Crate    "crate.json"
	PACKED
)
eagine_add_exe_analysis(app-018_uv_map_crate)

eagine_target_modules(
	app-018_uv_map_crate
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
	DESTINATION share/eagine/example/app
)
