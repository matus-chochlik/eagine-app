# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

file(GLOB EAGINE_APP_TEXTURES *.eagitexi *.eagitex)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-textures.eagizip"
	COMMAND ${ZIP_COMMAND} -q -j
				-9 "${CMAKE_CURRENT_BINARY_DIR}/app-textures.eagizip"
				${EAGINE_APP_TEXTURES}
	DEPENDS ${EAGINE_APP_TEXTURES})

add_custom_target("eagine-app-textures-eagizip"
	ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-textures.eagizip")

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/app-textures.eagizip"
	COMPONENT app-assets
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	DESTINATION share/eagine/assets)

