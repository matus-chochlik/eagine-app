# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(brick_torus)

add_executable(app-035_brick_torus main.cpp resources.cpp)
add_dependencies(app-035_brick_torus eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-035_brick_torus
	INPUT_TARGETS brick_torus bricks stones
	PACKED
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
