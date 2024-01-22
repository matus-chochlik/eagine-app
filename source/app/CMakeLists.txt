# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
add_custom_target(eagine-apps ALL)
add_subdirectory(model_viewer)
add_subdirectory(tiling_viewer)
add_subdirectory(resource_provider)

install(
	PROGRAMS make-voronoi-svg.py
	RENAME eagine-make-voronoi-svg
	DESTINATION bin
)

install(
	PROGRAMS obj-to-eagimesh.py
	RENAME eagine-obj-to-eagimesh
	DESTINATION bin
)

install(
	PROGRAMS blender-bake-light.py
	RENAME eagine-blender-bake-light
	DESTINATION bin
)

install(
	PROGRAMS blender-bake-to-vertex.py
	RENAME eagine-blender-bake-to-vertex
	DESTINATION bin
)

install(
	PROGRAMS blender-export-mesh.py
	RENAME eagine-blender-export-mesh
	DESTINATION bin
)

install(
	PROGRAMS blender-render-cubemap.py
	RENAME eagine-blender-render-cubemap
	DESTINATION bin
)

install(
	PROGRAMS blender-make-vertex-pivots.py
	RENAME eagine-blender-make-vertex-pivots
	DESTINATION bin
)

