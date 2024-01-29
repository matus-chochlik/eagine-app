/// @example app/020_bezier_patch/resources.hpp
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
class patch_program : public gl_program_resource {
public:
    patch_program(execution_context&);
    void set_projection(video_context&, orbiting_camera&);
    void set_wireframe_color(video_context&);
    void set_surface_color(video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location camera_matrix_loc;
    oglplus::uniform_location perspective_matrix_loc;
    oglplus::uniform_location color_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class patch_geometry {
public:
    void init(execution_context&);
    void clean_up(execution_context&);
    void draw(execution_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

private:
    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
