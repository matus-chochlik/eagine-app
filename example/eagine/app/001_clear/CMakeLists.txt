# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-001_clear main.cpp)
eagine_add_exe_analysis(app-001_clear)

add_dependencies(app-001_clear eagine-app-examples)

target_link_libraries(
	app-001_clear
	PUBLIC EAGine::App
)

set_target_properties(
	app-001_clear
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-001_clear
	DESTINATION share/eagine/example/app
)
