# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/eagimesh_operator_file_export.py"
	COMPONENT app-assets
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	DESTINATION share/eagine/assets/blender/addons)


