# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
# - testing headers or implementation files ------------------------------------
add_library(eagine-app-testing-headers INTERFACE)

set_property(
	TARGET eagine-app-testing-headers
	APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

