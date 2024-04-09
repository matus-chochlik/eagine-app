/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.sky_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// Cube maps
//------------------------------------------------------------------------------
sky_viewer_cube_maps::sky_viewer_cube_maps(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        load(
          "Default",
          url{"eagitex:///"
              "cube_map_sky?size=256&params=json%3A%2F%2F%2FSkyParams"},
          ctx,
          video,
          GL.texture_cube_map,
          GL.texture0);
    });
}
//------------------------------------------------------------------------------
auto sky_viewer_cube_maps::load_default(const url& locator) noexcept
  -> sky_viewer_cube_maps& {
    return *this;
}
//------------------------------------------------------------------------------
auto sky_viewer_cube_maps::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

