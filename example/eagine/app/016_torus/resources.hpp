/// @example app/016_torus/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class torus_program : public gl_program_resource {
public:
    torus_program(video_context&, resource_loader&);
    void set_projection(video_context&, orbiting_camera& camera);

    oglplus::program_input_bindings input_bindings;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry : public gl_geometry_and_bindings_resource {
public:
    torus_geometry(video_context&, resource_loader&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
