# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/wheelcart_1.json"
	"${CMAKE_CURRENT_BINARY_DIR}/wheelcart_1.json"
)

oglplus_add_glsl_program(stencil_shadow_draw)
oglplus_add_glsl_program(stencil_shadow_mask)
add_executable(app-030_stencil_shadow main.cpp)
add_dependencies(app-030_stencil_shadow eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-030_stencil_shadow
	INPUT_TARGETS
		stencil_shadow_draw
		stencil_shadow_mask
		wheelcart_1_color
		wheelcart_1_aoccl
		cloudy_day
	PACKED
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
