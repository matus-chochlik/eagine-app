# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-037_lantern main.cpp resources.cpp)
add_dependencies(app-037_lantern eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-037_lantern
	RESOURCES
		DrawVert "draw_vertex.glsl"
		DrawFrag "draw_fragment.glsl"
		DrawProg "draw_prog.json"
		ScreenVert "screen_vertex.glsl"
		ScreenFrag "screen_fragment.glsl"
		ScreenProg "screen_prog.json"
		Pumpkin "pumpkin.json"
		PumpkinTex "pumpkin.eagitex"
		PumpkinImg "pumpkin.eagitexi"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-037_lantern)

eagine_target_modules(
	app-037_lantern
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-037_lantern
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-037_lantern
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)
