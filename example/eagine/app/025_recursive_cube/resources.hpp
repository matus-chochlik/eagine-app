/// @example app/025_recursive_cube/resources.hpp
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
#include <eagine/double_buffer.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/quantities.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class cube_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void set_texture(video_context&, oglplus::gl_types::int_type);
    void set_projection(video_context&, const oglplus::tmat<float, 4, 4, true>&);
    void update(execution_context&, video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void
    bind_tex_coord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location projection_loc;
    oglplus::uniform_location modelview_loc;
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location cube_tex_loc;

    radians_t<float> rad{0.F};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class cube_geometry {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void draw(video_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto normal_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

    static auto tex_coord_loc() noexcept {
        return oglplus::vertex_attrib_location{2};
    }

private:
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name tex_coords;
    oglplus::owned_buffer_name indices;

    std::vector<oglplus::shape_draw_operation> ops;
};
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
class cube_draw_buffers {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);

    auto back_fbo() const noexcept {
        return oglplus::framebuffer_name{objs.back().fbo};
    }

    auto front_tex_unit() const noexcept {
        return objs.front().tex_unit;
    }

    auto side() const noexcept {
        return tex_side;
    }

    void swap() {
        objs.swap();
    }

private:
    const oglplus::gl_types::sizei_type tex_side{512};

    struct _buffer_objects {
        oglplus::gl_types::int_type tex_unit{};
        oglplus::owned_texture_name tex{};
        oglplus::owned_renderbuffer_name rbo{};
        oglplus::owned_framebuffer_name fbo{};
    };

    double_buffer<_buffer_objects> objs{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif