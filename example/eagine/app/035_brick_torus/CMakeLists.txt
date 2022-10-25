# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-035_brick_torus main.cpp resources.cpp)
add_dependencies(app-035_brick_torus eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-035_brick_torus
	RESOURCES
		VertShader "vertex.glsl"
		GeomShader "geometry.glsl"
		FragShader "fragment.glsl"
		Program    "prog.json"
		BricksColr "bricks_color_hmap.eagitexi"
		BricksNMap "bricks_nmap_hmap.eagitexi"
		Bricks     "bricks.eagitex"
		StonesColr "stones_color_hmap.eagitexi"
		StonesNMap "stones_nmap_hmap.eagitexi"
		Stones     "stones.eagitex"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-035_brick_torus)

eagine_target_modules(
	app-035_brick_torus
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-035_brick_torus
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-035_brick_torus
	DESTINATION share/eagine/example/app
)
