/// @example app/021_cel_shading/resources.hpp
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
class cel_program : public gpu_program {
public:
    void init(video_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void set_modelview(execution_context&, video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::uniform_location projection_loc;
    oglplus::uniform_location modelview_loc;
    float shp_turns{0.F};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class icosahedron_geometry : public gl_geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
