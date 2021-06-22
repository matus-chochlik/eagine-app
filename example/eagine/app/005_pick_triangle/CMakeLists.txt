# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-005_pick_triangle main.cpp)
eagine_embed_target_resources(TARGET app-005_pick_triangle)
eagine_add_exe_analysis(app-005_pick_triangle)

add_dependencies(app-005_pick_triangle eagine-app-examples)

target_link_libraries(
	app-005_pick_triangle
	PUBLIC EAGine::App
)

set_target_properties(
	app-005_pick_triangle
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-005_pick_triangle
	DESTINATION share/eagine/example/app
)
