/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_BACKGROUND_PLAIN_HPP
#define EAGINE_APP_BACKGROUND_PLAIN_HPP

#include <eagine/oglplus/gl_api.hpp>
#include <eagine/oglplus/math/vector.hpp>

namespace eagine::app {

class background_color {
public:
    background_color(
      const oglplus::gl_api& glapi,
      const oglplus::vec4 ca) noexcept
      : _red{ca.x()}
      , _green{ca.y()}
      , _blue{ca.z()}
      , _alpha{ca.w()} {
        init(glapi);
    }

    background_color(
      const oglplus::gl_api& glapi,
      const oglplus::vec3 c) noexcept
      : _red{c.x()}
      , _green{c.y()}
      , _blue{c.z()}
      , _alpha{1.F} {
        init(glapi);
    }

    background_color(
      const oglplus::gl_api& glapi,
      float gray,
      float alpha) noexcept
      : background_color{glapi, {gray, gray, gray, alpha}} {}

    background_color(const oglplus::gl_api& glapi, float gray) noexcept
      : background_color{glapi, gray, 1.F} {}

    background_color(const oglplus::gl_api& glapi) noexcept
      : background_color{glapi, 0.5F} {}

    auto init(const oglplus::gl_api& glapi) noexcept -> background_color& {
        glapi.clear_color(_red, _green, _blue, _alpha);
        return *this;
    }

    auto clear(const auto& gl, const auto& GL) noexcept -> background_color& {
        gl.clear(GL.color_buffer_bit);
        return *this;
    }

    auto clear(const oglplus::gl_api& glapi) noexcept -> background_color& {
        const auto& [gl, GL] = glapi;
        return clear(gl, GL);
    }

private:
    float _red;
    float _green;
    float _blue;
    float _alpha;
};

class background_color_depth {
public:
    background_color_depth(
      const oglplus::gl_api& glapi,
      const oglplus::vec4 ca,
      const float depth) noexcept
      : _color{glapi, ca}
      , _depth{depth} {
        init(glapi);
    }

    background_color_depth(
      const oglplus::gl_api& glapi,
      const oglplus::vec3 c,
      const float depth) noexcept
      : _color{glapi, c}
      , _depth{depth} {
        init(glapi);
    }

    background_color_depth(
      const oglplus::gl_api& glapi,
      const float gray,
      const float alpha,
      const float depth) noexcept
      : _color{glapi, gray, alpha}
      , _depth{depth} {
        init(glapi);
    }

    background_color_depth(
      const oglplus::gl_api& glapi,
      const float gray,
      const float depth) noexcept
      : _color{glapi, gray}
      , _depth{depth} {
        init(glapi);
    }

    auto init(const oglplus::gl_api& glapi) noexcept
      -> background_color_depth& {
        _color.init(glapi);
        glapi.clear_depth(_depth);
        return *this;
    }

    auto clear(const auto& gl, const auto& GL) noexcept
      -> background_color_depth& {
        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        return *this;
    }

    auto clear(const oglplus::gl_api& glapi) noexcept
      -> background_color_depth& {
        const auto& [gl, GL] = glapi;
        return clear(gl, GL);
    }

private:
    background_color _color;
    float _depth{1.F};
};

} // namespace eagine::app

#endif
