# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-013_tiling main.cpp)

eagine_embed_target_resources(
	TARGET app-013_tiling
	INPUT_TARGETS
	tiles_r4_s1024_a6
	tileset_thicket16
	tileset_blocks16
	tileset_nodes16
	tileset_connections16
	PACKED
)
eagine_add_exe_analysis(app-013_tiling)

add_dependencies(app-013_tiling eagine-app-examples)

target_link_libraries(
	app-013_tiling
	PUBLIC EAGine::App
)

set_target_properties(
	app-013_tiling
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-013_tiling
	DESTINATION share/eagine/example/app
)
