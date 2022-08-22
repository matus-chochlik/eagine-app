# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
oglplus_add_glsl_program(bezier_patch)
add_executable(app-020_bezier_patch main.cpp resources.cpp)
add_dependencies(app-020_bezier_patch eagine-app-examples)
eagine_embed_target_resources(
	TARGET app-020_bezier_patch
	INPUT_TARGETS bezier_patch
	PACKED
)
eagine_add_exe_analysis(app-020_bezier_patch)

if(${WITH_MODULES})
	eagine_target_modules(
		app-020_bezier_patch
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-020_bezier_patch
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-020_bezier_patch
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-020_bezier_patch
	DESTINATION share/eagine/example/app
)
