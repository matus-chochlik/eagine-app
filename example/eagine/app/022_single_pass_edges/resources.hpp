/// @example app/022_single_pass_edges/resources.hpp
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
class edges_program : public gl_program_resource {
public:
    edges_program(execution_context&);
    void set_projection(execution_context&, orbiting_camera&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location camera_loc;
    oglplus::uniform_location vp_dim_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class icosahedron_geometry : public gl_geometry_and_bindings {
public:
    void init(execution_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
