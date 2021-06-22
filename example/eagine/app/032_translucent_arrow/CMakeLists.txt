# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_executable(app-032_translucent_arrow main.cpp resources.cpp)
eagine_embed_target_resources(TARGET app-032_translucent_arrow PACKED)
eagine_add_exe_analysis(app-032_translucent_arrow)

add_dependencies(app-032_translucent_arrow eagine-app-examples)

target_link_libraries(
	app-032_translucent_arrow
	PUBLIC EAGine::App
)

set_target_properties(
	app-032_translucent_arrow
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-032_translucent_arrow
	DESTINATION share/eagine/example/app
)
