# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
	PARTITION resource
	IMPORTS
		std
		eagine.core
		eagine.guiplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
	PARTITION background
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
	PARTITION backgrounds
	IMPORTS
		std background
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
	PARTITION texture
	IMPORTS
		std resource
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
	PARTITION textures
	IMPORTS
		std texture
		eagine.core
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.sky_viewer
	COMPONENT app-dev
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

eagine_add_license(app-sky-viewer)
eagine_add_debian_changelog(app-sky-viewer)

install(
	TARGETS eagine-app-sky-viewer
	COMPONENT app-sky-viewer
	RUNTIME DESTINATION bin)

install(
	DIRECTORY etc
	COMPONENT app-sky-viewer
	DESTINATION /)

eagine_add_package_conffiles(app-sky-viewer etc)

eagine_add_bash_completion(app-sky-viewer "other/eagine-app-sky-viewer.bco")

eagine_add_package_property(
	app-sky-viewer
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS},\${EAGINE_SSLPLUS_RUNTIME_PKGS},\${EAGINE_EGLPLUS_RUNTIME_PKGS},\${EAGINE_OGLPLUS_RUNTIME_PKGS},\${EAGINE_OALPLUS_RUNTIME_PKGS},\${EAGINE_APP_RUNTIME_PKGS}")

eagine_add_package_property(
	app-sky-viewer
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "Application for viewing sky-box textures.")

