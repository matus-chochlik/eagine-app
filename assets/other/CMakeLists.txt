# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

set(EAGINE_APP_OTHER
	"${CMAKE_CURRENT_SOURCE_DIR}/sky_hazy_sunrise.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/sky.json.txt")

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-other.eagizip"
	COMMAND ${ZIP_COMMAND} -q -j
				-9 "${CMAKE_CURRENT_BINARY_DIR}/app-other.eagizip"
				${EAGINE_APP_OTHER}
	DEPENDS ${EAGINE_APP_OTHER})

add_custom_target("eagine-app-other-eagizip"
	ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-other.eagizip")

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/app-other.eagizip"
	COMPONENT app-assets
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	DESTINATION share/eagine/assets)


