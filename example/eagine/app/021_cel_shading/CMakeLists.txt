# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-021_cel_shading main.cpp resources.cpp)
add_dependencies(app-021_cel_shading eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-021_cel_shading
	RESOURCES
		VertShader vertex.glsl
		GeomShader geometry.glsl
		FragShader fragment.glsl
		CelProg celprog.json
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-021_cel_shading)

eagine_target_modules(
	app-021_cel_shading
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-021_cel_shading
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-021_cel_shading
	DESTINATION share/eagine/example/app
)
