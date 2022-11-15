# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-026_halo main.cpp resources.cpp)
add_dependencies(app-026_halo eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-026_halo
	RESOURCES
		SurfVert "vertex_surface.glsl"
		SurfFrag "fragment_surface.glsl"
		SurfProg "prog_surface.json"
		HaloVert "vertex_halo.glsl"
		HaloGeom "geometry_halo.glsl"
		HaloFrag "fragment_halo.glsl"
		HaloProg "prog_halo.json"
		TwstdSphre "twisted_sphere.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-026_halo)

eagine_target_modules(
	app-026_halo
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-026_halo
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-026_halo
	DESTINATION share/eagine/example/app
)
