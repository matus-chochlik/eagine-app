# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
find_program(ZIP_COMMAND zip)

add_subdirectory(blender)
add_subdirectory(models)
add_subdirectory(tilings)
add_subdirectory(tilesets)
add_subdirectory(textures)
add_subdirectory(other)

eagine_add_license(app-assets)
eagine_add_debian_changelog(app-assets)

eagine_add_package_property(
	app-assets
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "")

eagine_add_package_property(
	app-assets
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "EAGine application assets package.")

eagine_add_package_property(
	app-assets
	GENERATOR DEBIAN
	VARIABLE PACKAGE_ARCHITECTURE
	VALUE "all")

