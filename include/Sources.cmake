# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
set(HEADERS
    eagine/app/camera.hpp
    eagine/app/context.hpp
    eagine/app/framedump_raw.hpp
    eagine/app/fwd.hpp
    eagine/app/input.hpp
    eagine/app/interface.hpp
    eagine/app/main.hpp
    eagine/app/openal_oalplus.hpp
    eagine/app/opengl_eglplus.hpp
    eagine/app/opengl_glfw3.hpp
    eagine/app/options.hpp
    eagine/app/state.hpp
    eagine/app/state_view.hpp
    eagine/app/types.hpp
)

set(PUB_INLS
    eagine/app/state.inl
)

set(LIB_INLS
    eagine/app/camera.inl
    eagine/app/context.inl
    eagine/app/framedump_raw.inl
    eagine/app/openal_oalplus.inl
    eagine/app/opengl_eglplus.inl
    eagine/app/opengl_glfw3.inl
    eagine/app/options.inl
)

