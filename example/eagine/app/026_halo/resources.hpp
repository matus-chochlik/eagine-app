/// @example app/026_halo/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
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
class surface_program : public gl_program_resource {
public:
    surface_program(execution_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
};
//------------------------------------------------------------------------------
class halo_program : public gl_program_resource {
public:
    halo_program(execution_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
    oglplus::uniform_location _camera_pos_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry : public gl_geometry_and_bindings_resource {
public:
    shape_geometry(execution_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
