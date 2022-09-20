# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/monkey.json"
	"${CMAKE_CURRENT_BINARY_DIR}/monkey.json"
)
oglplus_add_glsl_program(furry_torus_surface)
oglplus_add_glsl_program(furry_torus_hair)

add_executable(app-029_furry_torus main.cpp resources.cpp)
add_dependencies(app-029_furry_torus eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-029_furry_torus
	INPUT_TARGETS
		furry_torus_surface
		furry_torus_hair
		zebra_fur
		monkey
	PACKED
)
eagine_add_exe_analysis(app-029_furry_torus)

eagine_target_modules(
	app-029_furry_torus
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-029_furry_torus
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-029_furry_torus
	DESTINATION share/eagine/example/app
)
