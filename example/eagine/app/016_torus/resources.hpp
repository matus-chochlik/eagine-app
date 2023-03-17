/// @example app/016_torus/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define EAGINE_APP_EXAMPLE_RESOURCES_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class torus_program : public gl_program_resource {
public:
    torus_program(execution_context&);
    void set_projection(video_context&, orbiting_camera& camera);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry : public gl_geometry_and_bindings_resource {
public:
    torus_geometry(execution_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
