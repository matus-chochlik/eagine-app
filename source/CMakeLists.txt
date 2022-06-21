# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
if(${WITH_MODULES})
	add_subdirectory(modules)
endif()
add_subdirectory(lib)

add_library(EAGine::App INTERFACE IMPORTED GLOBAL)
target_link_libraries(EAGine::App INTERFACE eagine-app-headers)

target_link_libraries(
	EAGine::App
	INTERFACE
		eagine-app
)

get_target_property(EAGINE_CORE_LIBS EAGine::Core INTERFACE_LINK_LIBRARIES)

install(
	TARGETS
		eagine-app-imgui
		eagine-app-private-headers
		eagine-app-headers
		eagine-app
		EXPORT App
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
	PUBLIC_HEADER DESTINATION include
)

install(
	EXPORT App
	NAMESPACE EAGine::
	FILE App.cmake
	DESTINATION lib/cmake/EAGine
)
