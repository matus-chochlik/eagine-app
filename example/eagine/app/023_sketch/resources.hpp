/// @example app/023_sketch/resources.hpp
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
#include <eagine/app/geometry.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class sketch_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void prepare_frame(video_context&, orbiting_camera& camera, float t);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_coord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _model_loc;
    oglplus::uniform_location _view_loc;
    oglplus::uniform_location _projection_loc;
    oglplus::uniform_location _vp_dim_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry : public geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
class sketch_texture {
public:
    void init(execution_context&, video_context&);
    void clean_up(execution_context&, video_context&);

private:
    oglplus::owned_texture_name _tex;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
