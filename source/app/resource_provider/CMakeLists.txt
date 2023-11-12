# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

add_executable(
	eagine-app-resource-provider
	driver.cpp
	common.cpp
	embedded.cpp
	eagitexi_random.cpp
	eagitexi_provider.cpp
	eagitexi_single_color.cpp
	eagitexi_sphere_volume.cpp
	eagitexi_checks.cpp
	eagitexi_stripes.cpp
	text_lorem_ipsum.cpp
	main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-resource-provider
	RESOURCES
		EdgeNVert  "glsl/edges_normal_vert.glsl"
		EdgeNGeom  "glsl/edges_normal_geom.glsl"
		EdgeNFrag  "glsl/edges_normal_frag.glsl"
		EdgeNProg  "glsl/edges_normal_prog.json"
		TraficCone "models/traffic_cone.json"
		StairCube  "models/stair_cube.json"
		Jerrycan   "models/jerrycan.json"
		CheckerImg "textures/checker.eagitexi"
		CheckerTex "textures/checker.eagitex"
	PACKED
	ENABLE_SEARCH)

eagine_add_exe_analysis(eagine-app-resource-provider)

eagine_target_modules(
	eagine-app-resource-provider
	std
	eagine.core
	eagine.sslplus
	eagine.msgbus
	eagine.app)

set_target_properties(
	eagine-app-resource-provider
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-resource-provider
	DESTINATION bin)
