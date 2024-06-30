# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-003_checker main.cpp)
add_dependencies(app-003_checker eagine-app-examples)

eagine_embed_target_resources(TARGET app-003_checker)
eagine_add_exe_analysis(app-003_checker)

eagine_target_modules(
	app-003_checker
	std
	eagine.core
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-003_checker
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-003_checker
	COMPONENT app-examples
	DESTINATION share/eagine/example/app)

install(
	FILES app-003_checker.yaml
	COMPONENT app-examples
	DESTINATION /etc/eagine)

