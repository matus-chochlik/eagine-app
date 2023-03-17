/// @example app/031_tessellation/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
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
class sphere_program : public gl_program_resource {
public:
    sphere_program(execution_context&);
    void set_projection(video_context&, orbiting_camera&);

    void bind_offsets_block(video_context&, oglplus::gl_types::uint_type);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location camera_matrix_loc;
    oglplus::uniform_location camera_position_loc;
    oglplus::uniform_location viewport_dim_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class icosahedron_geometry {
public:
    void init(video_context&);
    void clean_up(video_context&);
    void draw(video_context&);

    static auto attrib_bindings() noexcept {
        return oglplus::vertex_attrib_bindings{
          shapes::vertex_attrib_kind::position};
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
