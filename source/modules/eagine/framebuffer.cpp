/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:framebuffer;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.oglplus;
import :context;

namespace eagine::app {
//------------------------------------------------------------------------------
export class framebuffer_configuration
  : public oglplus::framebuffer_configuration {
    using base = oglplus::framebuffer_configuration;

public:
    framebuffer_configuration(video_context& vc) noexcept
      : base{vc.gl_api()} {}
};
//------------------------------------------------------------------------------
export class offscreen_framebuffer : public oglplus::offscreen_framebuffer {
    using base = oglplus::offscreen_framebuffer;

public:
    auto init(
      video_context& vc,
      const oglplus::framebuffer_configuration& config,
      const span<const oglplus::texture_unit::value_type> tex_units)
      -> offscreen_framebuffer& {
        const auto [width, height] = vc.surface_size();
        base::init(vc.gl_api(), width, height, config, tex_units);
        return *this;
    }

    auto resize(
      video_context& vc,
      const oglplus::framebuffer_configuration& config,
      const span<const oglplus::gl_types::enum_type> tex_units)
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

