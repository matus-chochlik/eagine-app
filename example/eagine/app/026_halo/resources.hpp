/// @example app/026_halo/resources.hpp
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
class surface_program : public gl_program_resource {
public:
    surface_program(video_context&, resource_loader&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

    oglplus::program_input_bindings input_bindings;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
};
//------------------------------------------------------------------------------
class halo_program : public gl_program_resource {
public:
    halo_program(video_context&, resource_loader&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

    oglplus::program_input_bindings input_bindings;

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
    shape_geometry(video_context&, resource_loader&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
