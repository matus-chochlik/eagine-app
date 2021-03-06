/// @example app/026_halo/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/fwd.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/oglplus/shapes/geometry.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class surface_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
};
//------------------------------------------------------------------------------
class halo_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
    oglplus::uniform_location _camera_pos_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry
  : public oglplus::vertex_attrib_bindings
  , public oglplus::geometry {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void draw(video_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
