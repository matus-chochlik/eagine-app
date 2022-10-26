# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/wheelcart_1.json"
	"${CMAKE_CURRENT_BINARY_DIR}/wheelcart_1.json"
)

add_executable(app-030_stencil_shadow main.cpp)
add_dependencies(app-030_stencil_shadow eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-030_stencil_shadow
	INPUT_TARGETS
		wheelcart_1_color
		wheelcart_1_aoccl
		cloudy_day
	RESOURCES
		DrawVert  "draw_vertex.glsl"
		DrawFrag  "draw_fragment.glsl"
		DrawProg  "draw_prog.json"
		MaskVert  "mask_vertex.glsl"
		MaskGeom  "mask_geometry.glsl"
		MaskFrag  "mask_fragment.glsl"
		MaskProg  "mask_prog.json"
		Wheelcart "wheelcart_1.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-030_stencil_shadow)

eagine_target_modules(
	app-030_stencil_shadow
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-030_stencil_shadow
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-030_stencil_shadow
	DESTINATION share/eagine/example/app
)
