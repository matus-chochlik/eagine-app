# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
include(./Sources.cmake)
# - regular headers ------------------------------------------------------------
add_library(eagine-app-headers INTERFACE)

target_include_directories(
	eagine-app-headers
	INTERFACE
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

foreach(FILENAME ${HEADERS} ${PUB_INLS})
	set_property(
		TARGET eagine-app-headers
		APPEND PROPERTY PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
	)
endforeach()
# - private headers or implementation files ------------------------------------
add_library(eagine-app-private-headers INTERFACE)

target_include_directories(
	eagine-app-private-headers
	INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

foreach(FILENAME ${GEN_INLS})
	set_property(
		TARGET eagine-app-private-headers
		APPEND PROPERTY PRIVATE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}
	)
endforeach()
