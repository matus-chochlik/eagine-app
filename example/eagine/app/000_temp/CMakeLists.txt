# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-000_temp main.cpp)
add_dependencies(app-000_temp eagine-app-examples)

eagine_embed_target_resources(
	TARGET app-000_temp
	RESOURCES
		VertShader vertex.glsl
		FragShader fragment.glsl
		ShapeJson shape.json
	PACKED
	ENABLE_SEARCH
)
eagine_add_exe_analysis(app-000_temp)

eagine_target_modules(
	app-000_temp
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-000_temp
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-000_temp
	DESTINATION share/eagine/example/app
)
