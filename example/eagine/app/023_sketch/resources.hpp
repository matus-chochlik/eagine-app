/// @example app/023_sketch/resources.hpp
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
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class sketch_program : public gl_program_resource {
public:
    sketch_program(execution_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
    oglplus::uniform_location _vp_dim_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry : public gl_geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
class sketch_texture {
public:
    void init(video_context&);
    void clean_up(video_context&);

private:
    oglplus::owned_texture_name _tex;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
