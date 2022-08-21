# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-006_writing main.cpp)
add_dependencies(app-006_writing eagine-app-examples)

eagine_embed_target_resources(TARGET app-006_writing)
eagine_add_exe_analysis(app-006_writing)

if(${WITH_MODULES})
	eagine_target_modules(
		app-006_writing
		eagine.core
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-006_writing
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-006_writing
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-006_writing
	DESTINATION share/eagine/example/app
)
