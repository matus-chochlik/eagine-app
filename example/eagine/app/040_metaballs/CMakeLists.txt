# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-040_metaballs main.cpp resources.cpp)
add_dependencies(app-040_metaballs eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-040_metaballs
	RESOURCES
		FieldComp "comp_field.glsl"
		FieldProg "prog_field.json"
		MBallComp "comp_metaball.glsl"
		MBallProg "prog_metaball.json"
		SrfceVert  "vertex.glsl"
		SrfceGeom  "geometry.glsl"
		SrfceFrag  "fragment.glsl"
		SrfceProg  "prog_surface.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-040_metaballs)

eagine_target_modules(
	app-040_metaballs
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-040_metaballs
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-040_metaballs
	DESTINATION share/eagine/example/app
)
