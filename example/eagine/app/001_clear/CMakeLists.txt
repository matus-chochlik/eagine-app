# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-001_clear main.cpp)
add_dependencies(app-001_clear eagine-app-examples)

eagine_add_exe_analysis(app-001_clear)

eagine_target_modules(
	app-001_clear
	std
	eagine.core
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-001_clear
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-001_clear
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)
