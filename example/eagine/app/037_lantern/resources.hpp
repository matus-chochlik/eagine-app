/// @example app/037_lantern/resources.hpp
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
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/oglplus/shapes/generator.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class draw_program {
public:
    void init(video_context&);
    void use(video_context&);
    void clean_up(video_context&);
    void set_camera(video_context&, const orbiting_camera& camera);
    void set_candle_light(video_context&, oglplus::gl_types::float_type);
    void set_ambient_light(video_context&, oglplus::gl_types::float_type);
    void set_texture_unit(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_wrap_coord_location(
      video_context&,
      oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _candle_light_loc;
    oglplus::uniform_location _ambient_light_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
class screen_program {
public:
    void init(video_context&);
    void use(video_context&);
    void clean_up(video_context&);
    void set_screen_size(video_context& vc);
    void set_texture_unit(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_coord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _screen_size_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class pumpkin_geometry {
public:
    void init(video_context&);
    void use(video_context&);
    void draw(video_context&);
    void clean_up(video_context&);

    auto position_loc() const noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    auto normal_loc() const noexcept {
        return oglplus::vertex_attrib_location{1};
    }

    auto wrap_coord_loc() const noexcept {
        return oglplus::vertex_attrib_location{2};
    }

    auto tex_unit() const noexcept -> oglplus::gl_types::int_type {
        return 0;
    }

    auto bounding_sphere() const noexcept {
        return _bounding_sphere;
    }

private:
    oglplus::sphere _bounding_sphere;
    std::vector<oglplus::shape_draw_operation> _ops;
    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _normals;
    oglplus::owned_buffer_name _wrap_coords;
    oglplus::owned_buffer_name _indices;

    oglplus::owned_texture_name _tex{};
};
//------------------------------------------------------------------------------
class screen_geometry {
public:
    void init(video_context&);
    void use(video_context&);
    void draw(video_context&);
    void clean_up(video_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto coord_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

private:
    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _coords;

    std::vector<oglplus::shape_draw_operation> _ops{};
};
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
class draw_buffers {
public:
    void init(video_context&);
    void resize(video_context&);
    void clean_up(video_context&);

    void draw_off_screen(video_context&);
    void draw_on_screen(video_context&);

    auto tex_unit() const noexcept -> oglplus::gl_types::int_type {
        return 1;
    }

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
