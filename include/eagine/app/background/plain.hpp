/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_BACKGROUND_PLAIN_HPP
#define EAGINE_APP_BACKGROUND_PLAIN_HPP

#include <eagine/app/context.hpp>
#include <eagine/oglplus/gl_api.hpp>
#include <eagine/oglplus/math/vector.hpp>

namespace eagine::app {

class background_color {
public:
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

    background_color(float gray, float alpha) noexcept
      : background_color{{gray, gray, gray, alpha}} {}

    background_color(float gray) noexcept
      : background_color{gray, 1.F} {}

    background_color() noexcept
      : background_color{0.5F} {}

    auto setup(video_context& vc) noexcept -> background_color& {
        vc.gl_api().clear_color(_red, _green, _blue, _alpha);
        return *this;
    }

    auto clear(video_context& vc) noexcept -> background_color& {
        setup(vc);
        const auto& [gl, GL] = vc.gl_api();
        gl.clear(GL.color_buffer_bit);
        return *this;
    }

private:
    float _red;
    float _green;
    float _blue;
    float _alpha;
};

class background_color_depth {
public:
    background_color_depth(const oglplus::vec4 ca, const float depth) noexcept
      : _color{ca}
      , _depth{depth} {}

    background_color_depth(const oglplus::vec3 c, const float depth) noexcept
      : _color{c}
      , _depth{depth} {}

    background_color_depth(
      const float gray,
      const float alpha,
      const float depth) noexcept
      : _color{gray, alpha}
      , _depth{depth} {}

    background_color_depth(const float gray, const float depth) noexcept
      : _color{gray}
      , _depth{depth} {}

    auto setup(video_context& vc) noexcept -> background_color_depth& {
        _color.setup(vc);
        vc.gl_api().clear_depth(_depth);
        return *this;
    }

    auto clear(video_context& vc) noexcept -> background_color_depth& {
        setup(vc);
        const auto& [gl, GL] = vc.gl_api();
        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        return *this;
    }

private:
    background_color _color;
    float _depth{1.F};
};

} // namespace eagine::app

#endif
