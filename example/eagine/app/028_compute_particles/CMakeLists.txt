# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(compute_particles_draw)
oglplus_add_glsl_program(compute_particles_emit)

add_executable(app-028_compute_particles main.cpp resources.cpp)
add_dependencies(app-028_compute_particles eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-028_compute_particles
	INPUT_TARGETS compute_particles_draw compute_particles_emit
	PACKED
)
eagine_add_exe_analysis(app-028_compute_particles)

if(${WITH_MODULES})
	eagine_target_modules(
		app-028_compute_particles
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-028_compute_particles
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-028_compute_particles
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-028_compute_particles
	DESTINATION share/eagine/example/app
)
