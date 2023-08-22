# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

install(
	PROGRAMS make-voronoi-svg.py
	RENAME eagine-make-voronoi-svg
	DESTINATION bin
)

install(
	PROGRAMS blender-bake-light.py
	RENAME eagine-blender-bake-light
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

