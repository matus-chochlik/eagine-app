# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

add_executable(
	eagine-app-resource-provider
	driver.cpp
	common.cpp
	eagitexi_random.cpp
	eagitexi_provider.cpp
	eagitexi_single_color.cpp
	eagitexi_checks.cpp
	eagitexi_stripes.cpp
	text_lorem_ipsum.cpp
	main.cpp)

eagine_target_modules(
	eagine-app-resource-provider
	std
	eagine.core
	eagine.sslplus
	eagine.msgbus
	eagine.app)

set_target_properties(
	eagine-app-resource-provider
	PROPERTIES FOLDER "App")

install(
	TARGETS eagine-app-resource-provider
	DESTINATION bin)
