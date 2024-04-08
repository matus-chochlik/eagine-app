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
static int tex_unit = 0;
//------------------------------------------------------------------------------
// Cube maps
//------------------------------------------------------------------------------
sky_viewer_cube_maps::sky_viewer_cube_maps(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        const std::array<std::tuple<std::string, url>, 1> args{
          {{"Default",
            url{"eagitex:///"
                "cube_map_sky?size=256&params=json%3A%2F%2F%2FSkyParams"}}}};

        for(const auto& [name, loc] : args) {
            load(
              name,
              loc,
              ctx,
              video,
              GL.texture_cube_map,
              GL.texture0 + tex_unit++);
        }
    });
}
//------------------------------------------------------------------------------
auto sky_viewer_cube_maps::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

