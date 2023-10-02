# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#

add_executable(
	eagine-app-model-viewer
	resource.cpp
	background.cpp
	backgrounds.cpp
	geometry.cpp
	models.cpp
	program.cpp
	programs.cpp
	texture.cpp
	textures.cpp
	launchpad.cpp
	main.cpp)
add_dependencies(eagine-app-model-viewer eagine-apps)

eagine_embed_target_resources(
	TARGET eagine-app-model-viewer
	RESOURCES
		DfaultVert "glsl/default_vert.glsl"
		DfaultFrag "glsl/default_frag.glsl"
		DfaultProg "glsl/default_prog.json"
		Atr2ClrVrt "glsl/attrib_to_color_vert.glsl"
		Atr2ClrFrg "glsl/attrib_to_color_frag.glsl"
		Nml2ClrPrg "glsl/normal_to_color_prog.json"
		Tgt2ClrPrg "glsl/tangent_to_color_prog.json"
		Btg2ClrPrg "glsl/bitangent_to_color_prog.json"
		Wrp2ClrPrg "glsl/wrap_to_color_prog.json"
		EdgeNVert  "glsl/edges_normal_vert.glsl"
		EdgeNGeom  "glsl/edges_normal_geom.glsl"
		EdgeNFrag  "glsl/edges_normal_frag.glsl"
		EdgeNProg  "glsl/edges_normal_prog.json"
		EdgeUVVert "glsl/edges_wrap_vert.glsl"
		EdgeUVGeom "glsl/edges_wrap_geom.glsl"
		EdgeUVFrag "glsl/edges_wrap_frag.glsl"
		EdgeUVProg "glsl/edges_wrap_prog.json"
		TraficCone "models/traffic_cone.json"
		Jerrycan   "models/jerrycan.json"
		Hydrant    "models/hydrant.json"
		Pumpkin    "models/pumpkin.json"
		Guitar     "models/guitar.json"
		Magnet     "models/magnet.json"
		Crate1     "models/crate1.json"
		Stool      "models/stool.json"
		Arrow      "models/arrow.json"
		CheckerImg "textures/checker.eagitexi"
		CheckerTex "textures/checker.eagitex"
	PACKED
	ENABLE_SEARCH)
eagine_add_exe_analysis(eagine-app-model-viewer)

eagine_target_modules(
	eagine-app-model-viewer
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.guiplus
	eagine.app)

set_target_properties(
	eagine-app-model-viewer
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-model-viewer
	DESTINATION bin)
