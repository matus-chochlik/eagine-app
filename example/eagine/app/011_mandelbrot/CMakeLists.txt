# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(app-011_mandelbrot main.cpp)
add_dependencies(app-011_mandelbrot eagine-app-examples)

eagine_embed_target_resources(TARGET app-011_mandelbrot)
eagine_add_exe_analysis(app-011_mandelbrot)

eagine_target_modules(
	app-011_mandelbrot
	eagine.core
	eagine.oglplus
	eagine.app
)

set_target_properties(
	app-011_mandelbrot
	PROPERTIES FOLDER "Example/App"
)

install(
	TARGETS app-011_mandelbrot
	DESTINATION share/eagine/example/app
)
