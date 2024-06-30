# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

file(GLOB EAGINE_APP_TILINGS r*.txt)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-tilings.eagizip"
	COMMAND ${ZIP_COMMAND} -q -j
				-9 "${CMAKE_CURRENT_BINARY_DIR}/app-tilings.eagizip"
				${EAGINE_APP_TILINGS}
	DEPENDS ${EAGINE_APP_TILINGS})

add_custom_target("eagine-app-tilings-eagizip"
	ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-tilings.eagizip")

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/app-tilings.eagizip"
	COMPONENT app-assets
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	DESTINATION share/eagine/assets)

unset(EAGINE_APP_TILING_TRANSITIONS)
foreach(TILING ${EAGINE_APP_TILINGS})
	get_filename_component(TILING_NAME "${TILING}" NAME_WE)
	string(REGEX REPLACE
		"r[23456]_s\([0-9]\+\)_[0-9][0-9]" "\\1"
		TILING_SIDE "${TILING_NAME}")
	configure_file(tiling.eagitex.in ${TILING_NAME}.eagitex)
	list(APPEND EAGINE_APP_TILING_TRANSITIONS
		"${CMAKE_CURRENT_BINARY_DIR}/${TILING_NAME}.eagitex")
endforeach()

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-tiling-transitions.eagizip"
	COMMAND ${ZIP_COMMAND} -q -j
				-9 "${CMAKE_CURRENT_BINARY_DIR}/app-tiling-transitions.eagizip"
				${EAGINE_APP_TILING_TRANSITIONS}
				DEPENDS ${EAGINE_APP_TILING_TRANSITIONS})

add_custom_target("eagine-app-tiling-transitions-eagizip"
	ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-tiling-transitions.eagizip")

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/app-tiling-transitions.eagizip"
	COMPONENT app-assets
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	DESTINATION share/eagine/assets)

