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
	eagine.app
	PP_NAME APP
	INTERFACES app
	PARTITIONS
		types
		options
		input
		state
		interface
	SOURCES
		options_impl
		state_impl
)

