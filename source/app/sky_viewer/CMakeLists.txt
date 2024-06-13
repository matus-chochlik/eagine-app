# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.sky_viewer PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer PARTITION background
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer PARTITION backgrounds
	IMPORTS
		std background
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer PARTITION texture
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer PARTITION textures
	IMPORTS
		std texture
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	SOURCES
		resource
		background
		backgrounds
		texture
		textures
		launchpad
		sky_viewer
	IMPORTS
		std
		eagine.core
		eagine.shapes
		eagine.msgbus
		eagine.guiplus
		eagine.oglplus
		eagine.app)

add_executable(eagine-app-sky-viewer main.cpp)

eagine_embed_target_resources(
	TARGET eagine-app-sky-viewer
	RESOURCES
		BoostLcnse "../../../LICENSE_1_0.txt"
		BashCmpltn "other/eagine-app-sky-viewer.bco"
	PACKED
	ENABLE_SEARCH)
eagine_add_exe_analysis(eagine-app-sky-viewer)

eagine_target_modules(
	eagine-app-sky-viewer
	std
	eagine.core
	eagine.shapes
	eagine.oglplus
	eagine.app
	eagine.app.sky_viewer)

set_target_properties(
	eagine-app-sky-viewer
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-sky-viewer
	COMPONENT app-sky-viewer
	RUNTIME DESTINATION bin)
