# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

if(NOT TARGET EAGine::Core AND NOT TARGET eagine.core)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-core/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-core"
	)
	add_subdirectory(eagine-core EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET EAGine::Shapes AND NOT TARGET eagine.shapes)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-shapes/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-shapes"
	)
	add_subdirectory(eagine-shapes EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET EAGine::EGLplus AND NOT TARGET eagine.eglplus)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-eglplus/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-eglplus"
	)
	add_subdirectory(eagine-eglplus EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET EAGine::OGLplus AND NOT TARGET eagine.oglplus)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-oglplus/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-oglplus"
	)
	add_subdirectory(eagine-oglplus EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET EAGine::OALplus AND NOT TARGET eagine.oalplus)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-oalplus/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-oalplus"
	)
	add_subdirectory(eagine-oalplus EXCLUDE_FROM_ALL)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h")
	execute_process(
		COMMAND git submodule update --init
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
	)
endif()

add_library(
	eagine-app-imgui
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_tables.cpp
	imgui/backends/imgui_impl_glfw.cpp
	imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(
	eagine-app-imgui
	PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>"
)
target_compile_definitions(
	eagine-app-imgui
	PUBLIC EAGINE_APP_USE_IMGUI=1
)
if(UNIX AND NOT APPLE)
	target_link_libraries(
		eagine-app-imgui
		PUBLIC dl
	)
endif()
