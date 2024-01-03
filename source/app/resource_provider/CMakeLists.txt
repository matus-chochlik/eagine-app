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
	eagitexi_tiling.cpp
	eagitexi_single_color.cpp
	eagitexi_sphere_volume.cpp
	eagitexi_checks.cpp
	eagitexi_stripes.cpp
	text_lorem_ipsum.cpp
	text_resource_list.cpp
	special_args.cpp
	main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-resource-provider
	RESOURCES
		BoostLcnse "../../../LICENSE_1_0.txt"
		BashCmpltn "other/eagine-app-resource-provider.bco"
		EdgeDVert  "glsl/edges_default_vert.glsl"
		EdgeDGeom  "glsl/edges_default_geom.glsl"
		EdgeDFrag  "glsl/edges_default_frag.glsl"
		EdgeDProg  "glsl/edges_default_prog.json"
		EdgeNVert  "glsl/edges_normal_vert.glsl"
		EdgeNGeom  "glsl/edges_normal_geom.glsl"
		EdgeNFrag  "glsl/edges_normal_frag.glsl"
		EdgeNProg  "glsl/edges_normal_prog.json"
		TraficCone "models/traffic_cone.json"
		StairCube  "models/stair_cube.json"
		Jerrycan   "models/jerrycan.json"
		Pumpkin    "models/pumpkin.json"
		Hydrant    "models/hydrant.json"
		Guitar     "models/guitar.json"
		Magnet     "models/magnet.json"
		Crate1     "models/crate1.json"
		Stool      "models/stool.json"
		CheckerImg "textures/checker.eagitexi"
		CheckerTex "textures/checker.eagitex"
		YBStrpImg0 "textures/yb_stripes_0.eagitexi"
		YBStrpImg1 "textures/yb_stripes_1.eagitexi"
		YBStrpImg2 "textures/yb_stripes_2.eagitexi"
		YBStrpImg3 "textures/yb_stripes_3.eagitexi"
		YBStrpImg4 "textures/yb_stripes_4.eagitexi"
		YBStrpImg5 "textures/yb_stripes_5.eagitexi"
		YBStrpImg6 "textures/yb_stripes_6.eagitexi"
		YBStrpImg7 "textures/yb_stripes_7.eagitexi"
		YBStrpImg8 "textures/yb_stripes_8.eagitexi"
		YBStrpImg9 "textures/yb_stripes_9.eagitexi"
		YBStrpsTex "textures/yb_stripes.eagitex"
		TlngR4S64  "tilings/r4_s64.txt"
		TlngR4S128 "tilings/r4_s128.txt"
		TlngR4S256 "tilings/r4_s256.txt"
		TlngR4S512 "tilings/r4_s512.txt"
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
