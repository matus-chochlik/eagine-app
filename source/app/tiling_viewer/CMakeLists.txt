# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.tiling_viewer PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer PARTITION geometry
	IMPORTS
		std resource
		eagine.core
		eagine.shapes
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer PARTITION models
	IMPORTS
		std geometry
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer PARTITION program
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer PARTITION programs
	IMPORTS
		std program
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.tiling_viewer
	SOURCES
		resource
		geometry
		models
		program
		programs
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
		TSBlk16Img "tilesets/blocks16.eagitexi"
		TSCon16Img "tilesets/connections16.eagitexi"
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
	DESTINATION bin)
