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

if(NOT TARGET EAGine::GUIplus AND NOT TARGET eagine.guiplus)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eagine-guiplus/CMakeLists.txt")
		unset(SUBMODULE_INIT_ARG)
	else()
		set(SUBMODULE_INIT_ARG --init)
	endif()
	execute_process(
		COMMAND git submodule update ${SUBMODULE_INIT_ARG}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/eagine-guiplus"
	)
	add_subdirectory(eagine-guiplus)
endif()
