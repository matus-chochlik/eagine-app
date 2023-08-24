# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(
	eagine-app-model-viewer
	resource.cpp
	background.cpp
	geometry.cpp
	program.cpp
	launchpad.cpp
	main.cpp)
add_dependencies(eagine-app-model-viewer eagine-apps)

eagine_embed_target_resources(
	TARGET eagine-app-model-viewer
	RESOURCES
		DfaultVert "glsl/default_vert.glsl"
		DfaultFrag "glsl/default_frag.glsl"
		DfaultProg "glsl/default_prog.json"
		Guitar     "models/guitar.json"
		TraficCone "models/traffic_cone.json"
	PACKED
	ENABLE_SEARCH)
eagine_add_exe_analysis(eagine-app-model-viewer)

eagine_target_modules(
	eagine-app-model-viewer
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app)

set_target_properties(
	eagine-app-model-viewer
	PROPERTIES FOLDER "App"
)

install(
	TARGETS eagine-app-model-viewer
	DESTINATION bin
)
