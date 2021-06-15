# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-010_newton main.cpp)
eagine_embed_target_resources(TARGET app-010_newton)
eagine_add_exe_analysis(app-010_newton)

add_dependencies(app-010_newton eagine-app-examples)

target_link_libraries(
	app-010_newton
	PUBLIC EAGine::App
)

set_target_properties(
	app-010_newton
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-010_newton
	DESTINATION share/eagine/example/app
)
