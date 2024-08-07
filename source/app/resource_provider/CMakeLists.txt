# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
add_executable(eagine-app-resource-provider main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-resource-provider
	RESOURCES
		BoostLcnse "../../../LICENSE_1_0.txt"
		BashCmpltn "other/eagine-app-resource-provider.bco"
		Manual     "other/eagine-app-resource-provider.man"
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
		ZebraImg   "textures/zebra_fur.eagitexi"
		ZebraTex   "textures/zebra_fur.eagitex"
		LvlGrdtTex "textures/level_gradient.eagitex"
		RandomTex  "textures/random.eagitex"
		OGLplusTex "textures/oglplus.eagitex"
		RandomCube "textures/random_cube.eagitex"
		ColorCube  "textures/color_cube.eagitex"
		PnkGryCube "textures/pink_gray_cube.eagitex"
		GreenCube  "textures/green_cube.eagitex"
		GrenCubImg "textures/green_cube.eagitexi"
		YBStrpCube "textures/yb_stripes_cube.eagitex"
		TNoise64   "textures/tiling_noise_64.eagitex"
		TNoise128  "textures/tiling_noise_128.eagitex"
		TNoise256  "textures/tiling_noise_256.eagitex"
		TNoise512  "textures/tiling_noise_512.eagitex"
		TlngR4S64  "tilings/r4_s64.txt"
		TlngR4S128 "tilings/r4_s128.txt"
		TlngR4S256 "tilings/r4_s256.txt"
		TlngR4S512 "tilings/r4_s512.txt"
		SkyParams  "other/default_sky.json"
		SkyParamsT "other/default_sky.json.txt"
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

eagine_add_license(app-resource-provider)
eagine_add_debian_changelog(app-resource-provider)
eagine_add_manual(app-resource-provider "other/eagine-app-resource-provider.man")
eagine_add_bash_completion(app-resource-provider "other/eagine-app-resource-provider.bco")

install(
	TARGETS eagine-app-resource-provider
	COMPONENT app-resource-provider
	RUNTIME DESTINATION bin)

install(
	DIRECTORY etc
	COMPONENT app-resource-provider
	DESTINATION /)

eagine_add_package_conffiles(app-resource-provider etc)

eagine_add_package_property(
	app-resource-provider
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS},\${EAGINE_APP_RUNTIME_PKGS}")

eagine_add_package_property(
	app-resource-provider
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "Resource provider message bus server.")

