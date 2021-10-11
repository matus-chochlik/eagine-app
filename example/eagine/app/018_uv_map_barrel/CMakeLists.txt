# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/barrel_1.json"
	"${CMAKE_CURRENT_BINARY_DIR}/barrel_1.json"
)

add_executable(app-018_uv_map_barrel main.cpp)

eagine_embed_target_resources(
	TARGET app-018_uv_map_barrel
	INPUT_TARGETS
	barrel_1_color
	barrel_1_aoccl
	barrel_1_rough
)
eagine_add_exe_analysis(app-018_uv_map_barrel)

add_dependencies(app-018_uv_map_barrel eagine-app-examples)

target_link_libraries(
	app-018_uv_map_barrel
	PUBLIC EAGine::App
)

set_target_properties(
	app-018_uv_map_barrel
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-018_uv_map_barrel
	DESTINATION share/eagine/example/app
)
