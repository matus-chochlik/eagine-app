# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-002_spectrum main.cpp)
add_dependencies(app-002_spectrum eagine-app-examples)

eagine_add_exe_analysis(app-002_spectrum)

if(${WITH_MODULES})
	eagine_target_modules(
		app-002_spectrum
		eagine.core
		eagine.oglplus
		eagine.app
	)
else()
	target_link_libraries(
		app-002_spectrum
		PUBLIC EAGine::App
	)
endif()

set_target_properties(
	app-002_spectrum
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-002_spectrum
	DESTINATION share/eagine/example/app
)

install(
	FILES app-002_spectrum.yaml
	DESTINATION etc/eagine
)
