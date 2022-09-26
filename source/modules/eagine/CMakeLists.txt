# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.app PARTITION types
	IMPORTS
		eagine.core.types
		eagine.core.reflection
)

eagine_add_module(
	eagine.app PARTITION options
	IMPORTS
		types
		eagine.core.types
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.valid_if
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION input
	IMPORTS
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.valid_if
		eagine.core.utility
		eagine.core.runtime
)

eagine_add_module(
	eagine.app PARTITION state
	IMPORTS
		eagine.core.types
		eagine.core.memory
		eagine.core.valid_if
		eagine.core.runtime
		eagine.core.utility
		eagine.core.units
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION interface
	IMPORTS
		types options input
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.utility
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION implementation
	IMPORTS
		interface
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION framedump_raw
	IMPORTS
		interface
		eagine.core.types
		eagine.core.memory
		eagine.core.reflection
		eagine.core.runtime
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION resource_loader
	IMPORTS
		eagine.core.types
		eagine.core.memory
		eagine.core.reflection
		eagine.core.utility
		eagine.core.runtime
		eagine.shapes
		eagine.oglplus
		eagine.msgbus
)

eagine_add_module(
	eagine.app PARTITION context
	IMPORTS
		interface options
		state input
		resource_loader
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.container
		eagine.core.utility
		eagine.core.main_ctx
		eagine.oglplus
		eagine.oalplus
)

eagine_add_module(
	eagine.app PARTITION main
	IMPORTS
		interface context
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.app PARTITION gpu_program
	IMPORTS
		context
		eagine.core.types
		eagine.core.memory
		eagine.core.resource
		eagine.oglplus
)

eagine_add_module(
	eagine.app PARTITION framebuffer
	IMPORTS
		context
		eagine.core.types
		eagine.core.memory
		eagine.oglplus
)

eagine_add_module(
	eagine.app PARTITION geometry
	IMPORTS
		context
		eagine.core.types
		eagine.core.memory
		eagine.shapes
		eagine.oglplus
)

eagine_add_module(
	eagine.app PARTITION camera
	IMPORTS
		state input context
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.math
		eagine.core.units
		eagine.core.utility
		eagine.core.valid_if
		eagine.oglplus
)

eagine_add_module(
	eagine.app PARTITION background
	IMPORTS
		camera context
		eagine.core.types
		eagine.core.memory
		eagine.core.c_api
		eagine.shapes
		eagine.oglplus
)

eagine_add_module(
	eagine.app
	PP_NAME APP
	INTERFACES app
	PARTITIONS
		types
		options
		input
		state
		interface
		implementation
		framedump_raw
		resource_loader
		context
		main
		gpu_program
		framebuffer
		geometry
		camera
		background
	SOURCES
		options_impl
		state_impl
		framedump_raw_impl
		openal_oalplus_impl
		opengl_eglplus_impl
		opengl_glfw3_impl
		resource_loader_impl
		context_impl
		camera_impl
		background_impl
	IMPORTS
		eagine.core.string
		eagine.core.container
		eagine.core.utility
		eagine.core.valid_if
		eagine.core.logging
		eagine.core.progress
		eagine.core.c_api
		eagine.core.main_ctx
		eagine.eglplus
		eagine.oglplus
		eagine.oalplus
)
target_link_libraries(
	eagine.app
	PUBLIC
	eagine-app-imgui
	EAGine::Deps::GLFW3
)
