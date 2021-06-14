# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
set(HEADERS
    eagine/application/camera.hpp
    eagine/application/context.hpp
    eagine/application/framedump_raw.hpp
    eagine/application/fwd.hpp
    eagine/application/input.hpp
    eagine/application/interface.hpp
    eagine/application/main.hpp
    eagine/application/opengl_eglplus.hpp
    eagine/application/opengl_glfw3.hpp
    eagine/application/options.hpp
    eagine/application/state.hpp
    eagine/application/state_view.hpp
    eagine/application/types.hpp
)

set(PUB_INLS
    eagine/application/state.inl
)

set(LIB_INLS
    eagine/application/camera.inl
    eagine/application/context.inl
    eagine/application/framedump_raw.inl
    eagine/application/opengl_eglplus.inl
    eagine/application/opengl_glfw3.inl
    eagine/application/options.inl
)

