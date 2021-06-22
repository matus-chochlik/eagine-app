# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-008_yaml_shape main.cpp)
eagine_embed_target_resources(TARGET app-008_yaml_shape)
eagine_add_exe_analysis(app-008_yaml_shape)

add_dependencies(app-008_yaml_shape eagine-app-examples)

target_link_libraries(
	app-008_yaml_shape
	PUBLIC EAGine::App
)

set_target_properties(
	app-008_yaml_shape
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-008_yaml_shape
	DESTINATION share/eagine/example/app
)
