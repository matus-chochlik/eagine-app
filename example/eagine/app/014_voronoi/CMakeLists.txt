# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-014_voronoi main.cpp resources.cpp)
eagine_embed_target_resources(TARGET app-014_voronoi)
eagine_add_exe_analysis(app-014_voronoi)

add_dependencies(app-014_voronoi eagine-app-examples)

target_link_libraries(
	app-014_voronoi
	PUBLIC EAGine::App
)

set_target_properties(
	app-014_voronoi
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-014_voronoi
	DESTINATION share/eagine/example/app
)
