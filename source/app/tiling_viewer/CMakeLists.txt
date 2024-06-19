# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION geometry
	IMPORTS
		std resource
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION models
	IMPORTS
		std geometry
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION program
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION programs
	IMPORTS
		std program
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION texture
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	PARTITION textures
	IMPORTS
		std texture
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	COMPONENT app-dev
	SOURCES
		resource
		geometry
		models
		program
		programs
		texture
		textures
		launchpad
		tiling_viewer
	IMPORTS
		std
		eagine.core
		eagine.shapes
		eagine.msgbus
		eagine.guiplus
		eagine.oglplus
		eagine.app)

add_executable(eagine-app-tiling-viewer main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-tiling-viewer
	RESOURCES
		BoostLcnse "../../../LICENSE_1_0.txt"
		BashCmpltn "other/eagine-app-tiling-viewer.bco"
		DfaultVert "glsl/default_vert.glsl"
		DfaultFrag "glsl/default_frag.glsl"
		DfaultProg "glsl/default_prog.json"
		Tlg1024Img "tilings/tiling_1024.eagitexi"
		Tlg1024Tex "tilings/tiling_1024.eagitex"
		TrnsPCBImg "transitions/pcb.eagitexi"
		TrnsPCBTex "transitions/pcb.eagitex"
		TrnsCkrTex "transitions/checker.eagitex"
		TSBlk16Img "tilesets/blocks16.eagitexi"
		TSBlk16Tex "tilesets/blocks16.eagitex"
		TSNds16Img "tilesets/nodes16.eagitexi"
		TSNds16Tex "tilesets/nodes16.eagitex"
		TSCon16Img "tilesets/connections16.eagitexi"
		TSCon16Tex "tilesets/connections16.eagitex"
		TSPCBImg   "tilesets/pcb.eagitexi"
		TSPCBTex   "tilesets/pcb.eagitex"
	PACKED
	ENABLE_SEARCH)
eagine_add_exe_analysis(eagine-app-tiling-viewer)

eagine_target_modules(
	eagine-app-tiling-viewer
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
	eagine.app.tiling_viewer)

set_target_properties(
	eagine-app-tiling-viewer
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-tiling-viewer
	COMPONENT app-tiling-viewer
	RUNTIME DESTINATION bin)

install(
	DIRECTORY etc
	COMPONENT app-tiling-viewer
	DESTINATION /)

eagine_add_package_property(
	app-tiling-viewer
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS}")

eagine_add_package_property(
	app-tiling-viewer
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "Application for viewing tiling and tileset textures.")

