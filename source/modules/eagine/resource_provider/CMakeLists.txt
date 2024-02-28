# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.app.resource_provider PARTITION external_apis
	IMPORTS
		std
		eagine.core
		eagine.eglplus
		eagine.oglplus)

eagine_add_module(
	eagine.app.resource_provider PARTITION driver
	IMPORTS
		std
		eagine.core
		eagine.msgbus
		eagine.app)

eagine_add_module(
	eagine.app.resource_provider PARTITION gl_context
	IMPORTS
		driver std
		eagine.core
		eagine.msgbus
		eagine.eglplus
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.resource_provider PARTITION providers
	IMPORTS
		driver std
		eagine.core
		eagine.app)

eagine_add_module(
	eagine.app.resource_provider PARTITION common
	IMPORTS
		driver std
		gl_context
		eagine.core
		eagine.msgbus
		eagine.eglplus
		eagine.oglplus
		eagine.app)

eagine_add_module(
	eagine.app.resource_provider PARTITION eagitexi_provider
	IMPORTS
		driver providers std
		eagine.core
		eagine.msgbus)

eagine_add_module(
	eagine.app.resource_provider
	SOURCES
		external_apis
		gl_context
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
		eagitexi_transition
		eagitexi_sphere_volume
		eagitexi_cubemap_blur
		text_lorem_ipsum
		text_resource_list
		resource_provider
	IMPORTS
		std
		eagine.core
		eagine.shapes
		eagine.msgbus
		eagine.sslplus
		eagine.eglplus
		eagine.oglplus
		eagine.app
	PRIVATE_LINK_LIBRARIES
		EAGine::Deps::Zip)
