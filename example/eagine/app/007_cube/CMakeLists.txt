# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-007_cube main.cpp)
eagine_embed_target_resources(TARGET app-007_cube)
eagine_add_exe_analysis(app-007_cube)

add_dependencies(app-007_cube eagine-app-examples)

target_link_libraries(
	app-007_cube
	PUBLIC EAGine::App
)

set_target_properties(
	app-007_cube
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-007_cube
	DESTINATION share/eagine/example/app
)
