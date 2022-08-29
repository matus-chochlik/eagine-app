# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
if(${WITH_MODULES})
	add_subdirectory(modules)
else()
	add_subdirectory(lib)
endif()

set(EAGINE_APP_EXPORT_TARGETS)

if(TARGET eagine-app)
	add_library(EAGine::App INTERFACE IMPORTED GLOBAL)
	target_link_libraries(
		EAGine::App
		INTERFACE
			eagine-app-headers
			eagine-app
	)
	list(
		APPEND EAGINE_APP_EXPORT_TARGETS
			eagine-app-imgui
			eagine-app-private-headers
			eagine-app-headers
			eagine-app
	)

	install(
		EXPORT App
		NAMESPACE EAGine::
		FILE App.cmake
		DESTINATION lib/cmake/EAGine
	)
endif()

install(
	TARGETS ${EAGINE_APP_EXPORT_TARGETS}
	EXPORT App
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
	PUBLIC_HEADER DESTINATION include
)

