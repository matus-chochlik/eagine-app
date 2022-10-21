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
class torus_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void set_projection(video_context&, orbiting_camera& camera);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_texcoord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry : public gl_geometry_and_bindings {
public:
    void init(execution_context&, video_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
