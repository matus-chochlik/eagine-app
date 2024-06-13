# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.model_viewer PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION background
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION backgrounds
	IMPORTS
		std background
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION geometry
	IMPORTS
		std resource
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION models
	IMPORTS
		std geometry
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION program
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION programs
	IMPORTS
		std program
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION texture
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer PARTITION textures
	IMPORTS
		std texture
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	SOURCES
		resource
		background
		backgrounds
		geometry
		models
		program
		programs
		texture
		textures
		launchpad
		model_viewer
	IMPORTS
		std
		eagine.core
		eagine.shapes
		eagine.msgbus
		eagine.guiplus
		eagine.oglplus
		eagine.app)

add_executable(eagine-app-model-viewer main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-model-viewer
	RESOURCES
		BoostLcnse "../../../LICENSE_1_0.txt"
		BashCmpltn "other/eagine-app-model-viewer.bco"
		DfaultVert "glsl/default_vert.glsl"
		DfaultFrag "glsl/default_frag.glsl"
		DfaultProg "glsl/default_prog.json"
		MetalVert  "glsl/metal_vert.glsl"
		MetalFrag  "glsl/metal_frag.glsl"
		MetalProg  "glsl/metal_prog.json"
		Atr2ClrVrt "glsl/attrib_to_color_vert.glsl"
		Atr2ClrFrg "glsl/attrib_to_color_frag.glsl"
		Nml2ClrPrg "glsl/normal_to_color_prog.json"
		Tgt2ClrPrg "glsl/tangent_to_color_prog.json"
		Btg2ClrPrg "glsl/bitangent_to_color_prog.json"
		Wrp2ClrPrg "glsl/wrap_to_color_prog.json"
		EdgeUVVert "glsl/edges_wrap_vert.glsl"
		EdgeUVGeom "glsl/edges_wrap_geom.glsl"
		EdgeUVFrag "glsl/edges_wrap_frag.glsl"
		EdgeUVProg "glsl/edges_wrap_prog.json"
		Arrow      "models/arrow.json"
		CheckerImg "textures/checker.eagitexi"
		CheckerTex "textures/checker.eagitex"
		CheckerCub "textures/checker_cube.eagitex"
		NoiseCube  "textures/noise_cube.eagitex"
	PACKED
	ENABLE_SEARCH)
eagine_add_exe_analysis(eagine-app-model-viewer)

eagine_target_modules(
	eagine-app-model-viewer
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
	eagine.app.model_viewer)

set_target_properties(
	eagine-app-model-viewer
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-model-viewer
	COMPONENT app-model-viewer
	RUNTIME DESTINATION bin)
