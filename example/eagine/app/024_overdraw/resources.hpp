/// @example app/024_overdraw/resources.hpp
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
class example;
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class draw_program : public gl_program_resource {
public:
    draw_program(example&);
    void set_projection(example&);

    void bind_position_location(example&, oglplus::vertex_attrib_location);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
class screen_program : public gl_program_resource {
public:
    screen_program(example&);

    void bind_position_location(example&, oglplus::vertex_attrib_location);
    void bind_tex_coord_location(example&, oglplus::vertex_attrib_location);

    void set_screen_size(example&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _screen_size_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry {
public:
    void init(example&);
    void draw(example&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

private:
    const int count = 8;

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _indices;
    oglplus::owned_buffer_name _offsets;

    std::vector<oglplus::shape_draw_operation> _ops{};
};
//------------------------------------------------------------------------------
class screen_geometry {
public:
    void init(example&);
    void draw(example&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto tex_coord_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

private:
    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _tex_coords;

    std::vector<oglplus::shape_draw_operation> _ops{};
};
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
class draw_buffers {
public:
    void init(example&);
    void resize(example&);

    void draw_offscreen(example&);
    void draw_onscreen(example&);

private:
    oglplus::gl_types::sizei_type _width{0};
    oglplus::gl_types::sizei_type _height{0};
    oglplus::owned_texture_name _tex{};
    oglplus::owned_renderbuffer_name _rbo{};
    oglplus::owned_framebuffer_name _fbo{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
