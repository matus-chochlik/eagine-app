# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-002_clear_gui main.cpp)
add_dependencies(app-002_clear_gui eagine-app-examples)

eagine_add_exe_analysis(app-002_clear_gui)

eagine_target_modules(
	app-002_clear_gui
	std
	eagine.core
	eagine.oglplus
	eagine.guiplus
	eagine.app
)

set_target_properties(
	app-002_clear_gui
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-002_clear_gui
	DESTINATION share/eagine/example/app
)
