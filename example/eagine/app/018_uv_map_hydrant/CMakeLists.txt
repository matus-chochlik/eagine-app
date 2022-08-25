# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/hydrant_1.json"
	"${CMAKE_CURRENT_BINARY_DIR}/hydrant_1.json"
)

add_executable(app-018_uv_map_hydrant main.cpp)
add_dependencies(app-018_uv_map_hydrant eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-018_uv_map_hydrant
	INPUT_TARGETS hydrant
	PACKED
)
eagine_add_exe_analysis(app-018_uv_map_hydrant)

if(${WITH_MODULES})
	eagine_target_modules(
		app-018_uv_map_hydrant
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-018_uv_map_hydrant
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-018_uv_map_hydrant
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-018_uv_map_hydrant
	DESTINATION share/eagine/example/app
)
