# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/monkey.json"
	"${CMAKE_CURRENT_BINARY_DIR}/monkey.json"
)
add_executable(app-032_translucent_arrow main.cpp resources.cpp)
add_dependencies(app-032_translucent_arrow eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-032_translucent_arrow
	RESOURCES
		DrawVert   "vertex_draw.glsl"
		DrawFrag   "fragment_draw.glsl"
		DrawProg   "draw_prog.json"
		DepthVert  "vertex_depth.glsl"
		DepthProg  "depth_prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-032_translucent_arrow)

eagine_target_modules(
	app-032_translucent_arrow
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-032_translucent_arrow
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-032_translucent_arrow
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)
