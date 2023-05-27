# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
configure_file(
	"${PROJECT_SOURCE_DIR}/assets/models/traffic_cone.json"
	"${CMAKE_CURRENT_BINARY_DIR}/traffic_cone.json"
)

add_executable(app-017_occlusion main.cpp)
add_dependencies(app-017_occlusion eagine-app-examples)

eagine_embed_target_resources(TARGET app-017_occlusion PACKED)
eagine_add_exe_analysis(app-017_occlusion)

eagine_target_modules(
	app-017_occlusion
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-017_occlusion
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-017_occlusion
	DESTINATION share/eagine/example/app
)
