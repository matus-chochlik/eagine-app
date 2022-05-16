/// @example app/022_single_pass_edges/resources.hpp
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
#include <eagine/app/gpu_program.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class edges_program : public glsl_program {
public:
    void init(video_context&);
    void set_projection(video_context&, orbiting_camera&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location vp_dim_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class icosahedron_geometry : public geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
