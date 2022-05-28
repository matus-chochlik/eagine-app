/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_FRAMEBUFFER_HPP
#define EAGINE_APP_FRAMEBUFFER_HPP

#include "context.hpp"
#include <eagine/oglplus/framebuffer.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class framebuffer_configuration : public oglplus::framebuffer_configuration {
    using base = oglplus::framebuffer_configuration;

public:
    framebuffer_configuration(video_context& vc) noexcept
      : base{vc.gl_api()} {}
};
//------------------------------------------------------------------------------
class offscreen_framebuffer : public oglplus::offscreen_framebuffer {
    using base = oglplus::offscreen_framebuffer;

public:
    auto init(
      video_context& vc,
      const oglplus::framebuffer_configuration& config,
      const span<const oglplus::gl_types::sizei_type> tex_units)
      -> offscreen_framebuffer& {
        const auto [width, height] = vc.surface_size();
        base::init(vc.gl_api(), width, height, config, tex_units);
        return *this;
    }

    auto resize(
      video_context& vc,
      const oglplus::framebuffer_configuration& config,
      const span<const oglplus::gl_types::sizei_type> tex_units)
      -> offscreen_framebuffer& {
        const auto [width, height] = vc.surface_size();
        base::resize(vc.gl_api(), width, height, config, tex_units);
        return *this;
    }

    auto clean_up(video_context& vc) -> offscreen_framebuffer& {
        base::clean_up(vc.gl_api());
        return *this;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
