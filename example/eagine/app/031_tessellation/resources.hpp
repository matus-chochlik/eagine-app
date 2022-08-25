/// @example app/031_tessellation/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

#if EAGINE_APP_MODULE
import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
#else
#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/fwd.hpp>
#include <eagine/app/gpu_program.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#endif

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class sphere_program : public glsl_program {
public:
    void init(video_context&);
    void set_projection(video_context&, orbiting_camera&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_offsets_block(video_context&, oglplus::gl_types::uint_type);

private:
    oglplus::uniform_location camera_matrix_loc;
    oglplus::uniform_location camera_position_loc;
    oglplus::uniform_location viewport_dim_loc;
    oglplus::uniform_block_index offset_blk_idx{0U};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class icosahedron_geometry {
public:
    void init(video_context&);
    void clean_up(video_context&);
    void draw(video_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto offsets_binding() noexcept {
        return 0U;
    }

private:
    const int count = 4;

    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name indices;
    oglplus::owned_buffer_name offsets;

    std::vector<oglplus::shape_draw_operation> ops;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
