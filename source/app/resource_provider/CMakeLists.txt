# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.app.resource_provider PARTITION special_args
	IMPORTS std eagine.core)

eagine_add_module(
	eagine.app.resource_provider PARTITION driver
	IMPORTS
		std
		eagine.core
		eagine.msgbus)

eagine_add_module(
	eagine.app.resource_provider PARTITION providers
	IMPORTS
		driver std
		eagine.core
		eagine.msgbus)

eagine_add_module(
	eagine.app.resource_provider PARTITION common
	IMPORTS
		driver std
		eagine.core
		eagine.msgbus)

eagine_add_module(
	eagine.app.resource_provider PARTITION eagitexi_provider
	IMPORTS
		driver providers std
		eagine.core
		eagine.msgbus)

eagine_add_module(
	eagine.app.resource_provider
	SOURCES
		special_args
		driver
		common
		file
		zip_archive
		embedded
		shapes
		eagitexi_provider
		eagitexi_single_color
		eagitexi_random
		eagitexi_checks
		eagitexi_stripes
		eagitexi_tiling
		eagitexi_sphere_volume
		text_lorem_ipsum
		text_resource_list
	IMPORTS
		std
		eagine.core
		eagine.shapes
		eagine.msgbus
	PRIVATE_LINK_LIBRARIES
		EAGine::Deps::Zip)

add_executable(eagine-app-resource-provider main.cpp)

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
		JackOLntrn "models/jack_o_lantern.json"
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
		LvlGrdtTex "textures/level_gradient.eagitex"
		OGLplusTex "textures/oglplus.eagitex"
		GreenCube  "textures/green_cube.eagitex"
		TNoise64   "textures/tiling_noise_64.eagitex"
		TNoise128  "textures/tiling_noise_128.eagitex"
		TNoise256  "textures/tiling_noise_256.eagitex"
		TNoise512  "textures/tiling_noise_512.eagitex"
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
	eagine.shapes
	eagine.sslplus
	eagine.msgbus
	eagine.app
	eagine.app.resource_provider)

set_target_properties(
	eagine-app-resource-provider
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-resource-provider
	DESTINATION bin)
