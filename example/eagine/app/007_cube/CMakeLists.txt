# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-007_cube main.cpp)
add_dependencies(app-007_cube eagine-app-examples)

eagine_embed_target_resources(TARGET app-007_cube)
eagine_add_exe_analysis(app-007_cube)

eagine_target_modules(
	app-007_cube
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-007_cube
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-007_cube
	DESTINATION share/eagine/example/app
)
