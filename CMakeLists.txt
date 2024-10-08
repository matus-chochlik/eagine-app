# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
cmake_minimum_required(VERSION 3.27)

project(
	EAGineApp
	LANGUAGES CXX
	HOMEPAGE_URL https://github.com/matus-chochlik/eagine-app)

if(NOT EAGINE_CORE_ROOT)
	set(EAGINE_CORE_ROOT ${PROJECT_SOURCE_DIR}/submodules/eagine-core)
endif()
if(NOT EAGINE_CORE_BINARY_ROOT)
	set(EAGINE_CORE_BINARY_ROOT ${PROJECT_BINARY_DIR}/submodules/eagine-core)
endif()
if(NOT EAGINE_OGLPLUS_ROOT)
	set(EAGINE_OGLPLUS_ROOT ${PROJECT_SOURCE_DIR}/submodules/eagine-oglplus)
endif()
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config)
list(APPEND CMAKE_MODULE_PATH ${EAGINE_CORE_ROOT}/config)
list(APPEND CMAKE_MODULE_PATH ${EAGINE_OGLPLUS_ROOT}/config)
file(STRINGS "${EAGINE_CORE_ROOT}/VERSION" EAGINE_VERSION)

include(CTest)
add_subdirectory(submodules)
include(EAGineCoreCommon)
include(EAGinePackageApp)
find_package(EAGineGLFW3)
find_package(EAGineLibZip)

add_subdirectory(assets)
add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(tools)
add_subdirectory(example)
