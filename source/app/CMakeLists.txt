# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
add_custom_target(eagine-apps ALL)
add_subdirectory(model_viewer)
add_subdirectory(tiling_viewer)
add_subdirectory(sky_viewer)
add_subdirectory(resource_provider)

foreach(APP
		make-voronoi-svg
		ogj-to-eagimesh
		blender-bake-light
		blender-bake-to-vertex
		blender-export-mesh
		blender-render-cubemap
		blender-make-vertex-pivots)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${APP}.py"
					--print-bash-completion
					"${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${APP}.py")

	add_custom_target("eagine-${APP}-bco"
		ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco")

	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		COMPONENT oglplus-apps
		RENAME eagine-${APP}
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
		DESTINATION share/bash-completion/completions)

	install(
		PROGRAMS ${APP}.py
		COMPONENT oglplus-apps
		RENAME eagine-${APP}
		DESTINATION bin)
endforeach()
