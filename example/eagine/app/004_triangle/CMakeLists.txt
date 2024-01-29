# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-004_triangle main.cpp)
add_dependencies(app-004_triangle eagine-app-examples)

eagine_embed_target_resources(TARGET app-004_triangle)
eagine_add_exe_analysis(app-004_triangle)

eagine_target_modules(
	app-004_triangle
	std
	eagine.core
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-004_triangle
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-004_triangle
	DESTINATION share/eagine/example/app
)
