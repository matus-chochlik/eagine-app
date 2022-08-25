# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-015_worley main.cpp resources.cpp)
add_dependencies(app-015_worley eagine-app-examples)

eagine_embed_target_resources(TARGET app-015_worley)
eagine_add_exe_analysis(app-015_worley)

if(${WITH_MODULES})
	eagine_target_modules(
		app-015_worley
		eagine.core
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-015_worley
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-015_worley
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-015_worley
	DESTINATION share/eagine/example/app
)
