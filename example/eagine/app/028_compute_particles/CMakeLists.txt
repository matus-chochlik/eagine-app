# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-028_compute_particles main.cpp resources.cpp)
add_dependencies(app-028_compute_particles eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-028_compute_particles
	RESOURCES
		VertShader "vertex.glsl"
		GeomShader "geometry.glsl"
		FragShader "fragment.glsl"
		CompShader "compute.glsl"
		DrawProg   "draw_prog.json"
		EmitProg   "emit_prog.json"
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-028_compute_particles)

eagine_target_modules(
	app-028_compute_particles
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-028_compute_particles
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-028_compute_particles
	DESTINATION share/eagine/example/app
)
