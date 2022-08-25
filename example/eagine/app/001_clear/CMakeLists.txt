# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-001_clear main.cpp)
add_dependencies(app-001_clear eagine-app-examples)

eagine_add_exe_analysis(app-001_clear)

if(${WITH_MODULES})
	eagine_target_modules(
		app-001_clear
		eagine.core
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-001_clear
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-001_clear
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-001_clear
	DESTINATION share/eagine/example/app
)
