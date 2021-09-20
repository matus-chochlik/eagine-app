# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

add_library(
	eagine-app
	options.cpp
	context.cpp
	camera.cpp
	framedump.cpp
	oalplus.cpp
	eglplus.cpp
	glfw3.cpp
)
eagine_add_lib_analysis(eagine-app)

target_compile_definitions(
	eagine-app PUBLIC EAGINE_APP_LIBRARY=1
)

target_include_directories(
	eagine-app
	PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(
	eagine-app
	PUBLIC
		EAGine::Core
		EAGine::Shapes
		EAGine::EGLplus
		EAGine::OGLplus
		EAGine::OALplus
		eagine-app-headers
	PRIVATE
		eagine-app-private-headers
)
