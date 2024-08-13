# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
add_custom_target(eagine-apps ALL)
add_subdirectory(model_viewer)
add_subdirectory(tiling_viewer)
add_subdirectory(sky_viewer)
add_subdirectory(resource_provider)

find_program(PYTHON3_COMMAND python3)

foreach(APP add-cubemap-blur make-voronoi-svg obj-to-eagimesh)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		COMMAND ${PYTHON3_COMMAND} "${CMAKE_CURRENT_SOURCE_DIR}/${APP}.py"
					--print-bash-completion
					"${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${APP}.py")

	add_custom_target("eagine-${APP}-bco"
		ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco")

	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/eagine-${APP}.bco"
		COMPONENT app-apps
		RENAME eagine-${APP}
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
		DESTINATION share/bash-completion/completions)

	install(
		PROGRAMS ${APP}.py
		COMPONENT app-apps
		RENAME eagine-${APP}
		DESTINATION bin)
endforeach()

foreach(APP
		blender-bake-light
		blender-bake-to-vertex
		blender-export-mesh
		blender-render-cubemap
		blender-make-vertex-pivots)

	install(
		PROGRAMS ${APP}.py
		COMPONENT app-apps
		RENAME eagine-${APP}
		DESTINATION bin)
endforeach()

eagine_add_license(app-apps)
eagine_add_debian_changelog(app-apps)

eagine_add_package_property(
	app-apps
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS},\${EAGINE_APP_RUNTIME_PKGS}")

eagine_add_package_property(
	app-apps
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "Collection of EAGine utility applications.")

