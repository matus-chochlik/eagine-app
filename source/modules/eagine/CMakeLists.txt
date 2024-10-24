# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION types
	IMPORTS
		std
		eagine.core.types
		eagine.core.reflection
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION options
	IMPORTS
		std types
		eagine.core.types
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.valid_if
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION input
	IMPORTS
		std
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.valid_if
		eagine.core.utility
		eagine.core.runtime)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION state
	IMPORTS
		std
		eagine.core.types
		eagine.core.memory
		eagine.core.valid_if
		eagine.core.runtime
		eagine.core.utility
		eagine.core.units
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION interface
	IMPORTS
		std types options input
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.utility
		eagine.core.main_ctx
		eagine.eglplus
		eagine.guiplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION implementation
	IMPORTS
		std interface
		eagine.core.types
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION framedump_raw
	IMPORTS
		std interface
		eagine.core.types
		eagine.core.memory
		eagine.core.reflection
		eagine.core.runtime
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION old_resource_loader
	IMPORTS
		std
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.serialization
		eagine.core.container
		eagine.core.value_tree
		eagine.core.utility
		eagine.core.runtime
		eagine.core.logging
		eagine.core.progress
		eagine.core.main_ctx
		eagine.shapes
		eagine.oglplus
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_loader
	IMPORTS
		std
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.container
		eagine.core.value_tree
		eagine.core.valid_if
		eagine.core.utility
		eagine.core.runtime
		eagine.core.logging
		eagine.core.progress
		eagine.core.main_ctx
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_valtree
	IMPORTS
		resource_loader
		std
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.value_tree
		eagine.core.utility
		eagine.core.runtime
		eagine.core.progress
		eagine.core.main_ctx
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_basic
	IMPORTS
		resource_loader
		std
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.utility
		eagine.core.runtime
		eagine.core.progress
		eagine.core.main_ctx
		eagine.msgbus
		eagine.shapes
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_gl
	IMPORTS
		resource_loader
		std
		eagine.core.types
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.utility
		eagine.core.runtime
		eagine.core.progress
		eagine.core.main_ctx
		eagine.msgbus
		eagine.shapes
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_mapped
	IMPORTS
		resource_loader
		resource_valtree
		std
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.identifier
		eagine.core.value_tree
		eagine.core.serialization
		eagine.core.reflection
		eagine.core.valid_if
		eagine.core.utility
		eagine.core.runtime
		eagine.core.progress
		eagine.core.main_ctx
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION context
	IMPORTS
		std interface options
		types state input
		resource_loader
		old_resource_loader
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.container
		eagine.core.utility
		eagine.core.main_ctx
		eagine.eglplus
		eagine.oglplus
		eagine.oalplus
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION main
	IMPORTS
		std interface context
		eagine.core.types
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION framebuffer
	IMPORTS
		std context
		eagine.core.types
		eagine.core.memory
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION geometry
	IMPORTS
		std context
		eagine.core.types
		eagine.core.memory
		eagine.shapes
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION camera
	IMPORTS
		std state input context types
		eagine.core.types
		eagine.core.memory
		eagine.core.identifier
		eagine.core.math
		eagine.core.units
		eagine.core.utility
		eagine.core.valid_if
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION background
	IMPORTS
		std camera context types
		eagine.core.types
		eagine.core.memory
		eagine.core.c_api
		eagine.shapes
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION loaded_resource
	IMPORTS
		std context geometry
		old_resource_loader
		eagine.core.types
		eagine.core.math
		eagine.core.memory
		eagine.core.string
		eagine.core.utility
		eagine.core.runtime
		eagine.core.reflection
		eagine.core.value_tree
		eagine.shapes
		eagine.oglplus
		eagine.msgbus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION old_resource_manager
	IMPORTS
		std loaded_resource geometry
		context old_resource_loader
		eagine.core.types
		eagine.core.math
		eagine.core.utility
		eagine.core.runtime
		eagine.core.container
		eagine.core.value_tree
		eagine.oglplus)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	PARTITION resource_manager
	IMPORTS
		std context
		resource_loader
		resource_mapped
		eagine.core.types
		eagine.core.utility
		eagine.core.runtime
		eagine.core.valid_if
		eagine.core.container
		eagine.core.identifier
		eagine.core.main_ctx)

eagine_add_module(
	eagine.app
	COMPONENT app-dev
	SOURCES
		options
		state
		geometry
		framedump_raw
		openal_oalplus
		opengl_eglplus
		opengl_glfw3
		old_resource_loader
		resource_loader
		resource_valtree
		resource_basic
		resource_gl_shader
		resource_manager
		resource_builders
		resource_gl_builders
		context
		camera
		background
	IMPORTS
		std
		eagine.core.types
		eagine.core.memory
		eagine.core.math
		eagine.core.string
		eagine.core.identifier
		eagine.core.reflection
		eagine.core.container
		eagine.core.utility
		eagine.core.runtime
		eagine.core.valid_if
		eagine.core.value_tree
		eagine.core.logging
		eagine.core.progress
		eagine.core.units
		eagine.core.c_api
		eagine.core.main_ctx
		eagine.shapes
		eagine.eglplus
		eagine.oglplus
		eagine.oalplus
		eagine.guiplus
		eagine.msgbus
	PRIVATE_LINK_LIBRARIES
		EAGine::Deps::GLFW3)

eagine_add_module_tests(
	eagine.app
	UNITS
		old_resource_loader
		resource_loader_basic
		resource_loader_gl
		resource_manager
	IMPORTS
		eagine.core
		eagine.msgbus
		eagine.app
	PRIVATE_LINK_LIBRARIES
		eagine-app-testing-headers
		EAGine::Deps::GLFW3)

eagine_embed_target_resources(
	TARGET test.eagine.app.resource_loader_basic
	RESOURCES
		TestText   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_text.txt"
		TestURLs   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_urls.txt"
		TestInts   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_ints.json"
		TestVec3   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_vec3.json"
		TestMat4   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_mat4.json"
		TestThdn1  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_thdn1.json"
		TestThdn2  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_thdn2.json"
		TestPath   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_path.json"
		TestMesh   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_mesh.json"
		TestFragS  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag1.glsl"
		TestFragE1 "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag1.eagishdr"
		TestFragE2 "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag2.eagishdr"
	PACKED ENABLE_SEARCH)

set_tests_properties(execute-test.eagine.app.resource_loader_basic PROPERTIES COST 10)


eagine_embed_target_resources(
	TARGET test.eagine.app.resource_loader_gl
	RESOURCES
		TestCheckr "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_checker.glsl"
		TestMndbrt "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_mandelbrot.glsl"
		TestIncls  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_includes.txt"
		TestFrag1  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag1.glsl"
		TestFrag2  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag2.glsl"
		TestFragE1 "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag1.eagishdr"
		TestFragE2 "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_frag2.eagishdr"
	PACKED ENABLE_SEARCH)

set_tests_properties(execute-test.eagine.app.resource_loader_gl PROPERTIES COST 10)


eagine_embed_target_resources(
	TARGET test.eagine.app.resource_manager
	RESOURCES
		TestText   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_text.txt"
	PACKED ENABLE_SEARCH)

set_tests_properties(execute-test.eagine.app.resource_manager PROPERTIES COST 10)

eagine_embed_target_resources(
	TARGET test.eagine.app.old_resource_loader
	RESOURCES
		TestText   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_text.txt"
		TestURLs   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_urls.txt"
		TestInts   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_ints.json"
		TestVec3   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_vec3.json"
		TestMat4   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_mat4.json"
		TestThdn1  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_thdn1.json"
		TestThdn2  "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_thdn2.json"
		TestPath   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_path.json"
		TestMesh   "${CMAKE_CURRENT_SOURCE_DIR}/resources/test_mesh.json"
	PACKED ENABLE_SEARCH)

set_tests_properties(execute-test.eagine.app.old_resource_loader PROPERTIES COST 10)

add_subdirectory(resource_provider)

eagine_add_license(app-dev)
eagine_add_debian_changelog(app-dev)

eagine_add_package_property(
	app-dev
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "eagine-core-dev (>= ${EAGINE_VERSION}),eagine-sslplus-dev (>= ${EAGINE_VERSION}),eagine-msgbus-dev (>= ${EAGINE_VERSION}),eagine-shapes-dev (>= ${EAGINE_VERSION}),eagine-guiplus-dev (>= ${EAGINE_VERSION}),eagine-eglplus-dev (>= ${EAGINE_VERSION}),eagine-oglplus-dev (>= ${EAGINE_VERSION}),eagine-oalplus-dev (>= ${EAGINE_VERSION})")

eagine_add_package_property(
	app-dev
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "High-level C++ utilities for implementing multimedia applications.")

