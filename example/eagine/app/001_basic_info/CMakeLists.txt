# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-001_basic_info main.cpp)
eagine_add_exe_analysis(app-001_basic_info)

add_dependencies(app-001_basic_info eagine-app-examples)

target_link_libraries(
	app-001_basic_info
	PUBLIC EAGine::App
)

set_target_properties(
	app-001_basic_info
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-001_basic_info
	DESTINATION share/eagine/example/app
)
