# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-012_atomic_counters main.cpp)
add_dependencies(app-012_atomic_counters eagine-app-examples)

eagine_embed_target_resources(TARGET app-012_atomic_counters PACKED)
eagine_add_exe_analysis(app-012_atomic_counters)

eagine_target_modules(
	app-012_atomic_counters
	eagine.core
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-012_atomic_counters
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-012_atomic_counters
	DESTINATION share/eagine/example/app
)
