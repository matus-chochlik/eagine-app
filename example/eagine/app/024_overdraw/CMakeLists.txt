# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-024_overdraw main.cpp resources.cpp)
add_dependencies(app-024_overdraw eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-024_overdraw
	RESOURCES
		DrawVert   "vs_draw.glsl"
		DrawFrag   "fs_draw.glsl"
		DrawProg   "draw_prog.json"
		ScreenVert "vs_screen.glsl"
		ScreenFrag "fs_screen.glsl"
		ScreenProg "screen_prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-024_overdraw)

eagine_target_modules(
	app-024_overdraw
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-024_overdraw
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-024_overdraw
	DESTINATION share/eagine/example/app
)
