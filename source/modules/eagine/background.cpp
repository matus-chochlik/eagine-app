/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:background;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.c_api;
import eagine.shapes;
import eagine.oglplus;
import :context;
import :camera;

namespace eagine::app {
//------------------------------------------------------------------------------
export class background_color {
public:
    using float_type = oglplus::gl_types::float_type;

    background_color(const oglplus::vec4 ca) noexcept
      : _red{ca.x()}
      , _green{ca.y()}
      , _blue{ca.z()}
      , _alpha{ca.w()} {}

    background_color(const oglplus::vec3 c) noexcept
      : _red{c.x()}
      , _green{c.y()}
      , _blue{c.z()}
      , _alpha{1.F} {}

    background_color(float_type gray, float_type alpha) noexcept
      : background_color{{gray, gray, gray, alpha}} {}

    background_color(float_type gray) noexcept
      : background_color{gray, 1.F} {}

    background_color() noexcept
      : background_color{0.5F} {}

    auto setup(video_context& vc) noexcept -> background_color& {
        vc.with_gl(
          [this](auto& gl) { gl.clear_color(_red, _green, _blue, _alpha); });
        return *this;
    }

    auto clear(video_context& vc) noexcept -> background_color& {
        setup(vc);
        vc.with_gl([](auto& gl, auto& GL) { gl.clear(GL.color_buffer_bit); });
        return *this;
    }

private:
    float_type _red;
    float_type _green;
    float_type _blue;
    float_type _alpha;
};
//------------------------------------------------------------------------------
export class background_color_depth {
public:
    using float_type = oglplus::gl_types::float_type;

    background_color_depth(
      const oglplus::vec4 ca,
      const float_type depth) noexcept
      : _color{ca}
      , _depth{depth} {}

    background_color_depth(
      const oglplus::vec3 c,
      const float_type depth) noexcept
      : _color{c}
      , _depth{depth} {}

    background_color_depth(
      const float_type gray,
      const float_type alpha,
      const float_type depth) noexcept
      : _color{gray, alpha}
      , _depth{depth} {}

    background_color_depth(
      const float_type gray,
      const float_type depth) noexcept
      : _color{gray}
      , _depth{depth} {}

    auto setup(video_context& vc) noexcept -> background_color_depth& {
        _color.setup(vc);
        vc.with_gl([this](auto& gl) { gl.clear_depth(_depth); });
        return *this;
    }

    auto clear(video_context& vc) noexcept -> background_color_depth& {
        setup(vc);
        vc.with_gl([this](auto& gl, auto& GL) {
            gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        });
        return *this;
    }

private:
    background_color _color;
    float_type _depth{1.F};
};
//------------------------------------------------------------------------------
export class background_icosahedron {
public:
    using float_type = oglplus::gl_types::float_type;

    background_icosahedron(
      video_context& vc,
      const oglplus::vec4 edge_color,
      const oglplus::vec4 face_color,
      const float_type depth) noexcept;

    auto clean_up(video_context& vc) noexcept -> background_icosahedron&;

    auto clear(video_context& vc, const orbiting_camera& camera) noexcept
      -> background_icosahedron&;

private:
    void _init(auto&, auto&, auto&) noexcept;
    void _clean_up(auto&) noexcept;

    oglplus::vec4 _ecolor;
    oglplus::vec4 _fcolor;
    float_type _depth;

    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _scale_loc;
    oglplus::uniform_location _color_loc;

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _indices;

    std::vector<oglplus::shape_draw_operation> _ops;
};
//------------------------------------------------------------------------------
export class background_skybox {
public:
    background_skybox(
      video_context&,
      oglplus::gl_types::enum_type tex_unit) noexcept;

    auto clean_up(video_context& vc) noexcept -> background_skybox&;

    auto clear(video_context& vc, const orbiting_camera& camera) noexcept
      -> background_skybox&;

private:
    void _init(auto&, auto&, auto&) noexcept;
    void _clean_up(auto&) noexcept;

    oglplus::gl_types::enum_type _tex_unit{0};

    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _scale_loc;
    oglplus::uniform_location _tex_loc;

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _coords;
    oglplus::owned_buffer_name _indices;

    std::vector<oglplus::shape_draw_operation> _ops;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
