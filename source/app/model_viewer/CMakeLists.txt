# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION background
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION backgrounds
	IMPORTS
		std background
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION geometry
	IMPORTS
		std resource
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION models
	IMPORTS
		std geometry
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION program
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION programs
	IMPORTS
		std program
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION texture
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
	PARTITION textures
	IMPORTS
		std texture
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.model_viewer
	COMPONENT app-dev
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

eagine_add_license(app-model-viewer)
eagine_add_debian_changelog(app-model-viewer)

install(
	TARGETS eagine-app-model-viewer
	COMPONENT app-model-viewer
	RUNTIME DESTINATION bin)

install(
	DIRECTORY etc
	COMPONENT app-model-viewer
	DESTINATION /)

eagine_add_package_conffiles(app-model-viewer etc)

eagine_add_bash_completion(app-model-viewer "other/eagine-app-model-viewer.bco")

eagine_add_package_property(
	app-model-viewer
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS},\${EAGINE_APP_RUNTIME_PKGS}")

eagine_add_package_property(
	app-model-viewer
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "Application for viewing 3D models.")

