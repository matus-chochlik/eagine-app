# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#

# add a target for the examples
add_custom_target(eagine-app-examples ALL)
set_target_properties(
	eagine-app-examples
	PROPERTIES FOLDER "Example/App"
)

add_subdirectory(001_basic_info)
add_subdirectory(001_clear)
add_subdirectory(002_clear_gui)
add_subdirectory(002_spectrum)
add_subdirectory(003_checker)
add_subdirectory(004_triangle)
add_subdirectory(005_pick_triangle)
add_subdirectory(006_writing)
add_subdirectory(007_cube)
add_subdirectory(008_yaml_shape)
add_subdirectory(009_json_shape)
add_subdirectory(010_newton)
add_subdirectory(011_mandelbrot)
add_subdirectory(012_atomic_counters)
add_subdirectory(013_tiling)
add_subdirectory(014_voronoi)
add_subdirectory(015_worley)
add_subdirectory(016_torus)
add_subdirectory(017_occlusion)
add_subdirectory(018_uv_map_crate)
add_subdirectory(018_uv_map_barrel)
add_subdirectory(018_uv_map_hydrant)
add_subdirectory(019_scaled_cubes)
add_subdirectory(020_bezier_patch)
add_subdirectory(021_cel_shading)
add_subdirectory(022_single_pass_edges)
add_subdirectory(023_sketch)
add_subdirectory(024_overdraw)
add_subdirectory(025_recursive_cube)
add_subdirectory(026_halo)
add_subdirectory(027_labeled_shapes)
add_subdirectory(028_compute_particles)
add_subdirectory(029_furry_torus)
add_subdirectory(030_stencil_shadow)
add_subdirectory(031_tessellation)
add_subdirectory(032_translucent_arrow)
add_subdirectory(034_shape_tessellation)
add_subdirectory(035_brick_torus)
add_subdirectory(037_lantern)
add_subdirectory(040_metaballs)

eagine_add_package_property(
	app-examples
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS},\${EAGINE_APP_RUNTIME_PKGS}")

eagine_add_package_property(
	app-examples
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "EAGine application examples.")

