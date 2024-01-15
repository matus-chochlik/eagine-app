/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.model_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// Cube maps
//------------------------------------------------------------------------------
model_viewer_cube_maps::model_viewer_cube_maps(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        if(auto arg{ctx.main_context().args().find("--cube-map")}) {
            load(arg, ctx, video, GL.texture_cube_map, 0);
        } else {
            load(
              "Default",
              url{"eagitex:///CheckerCub"},
              ctx,
              video,
              GL.texture_cube_map,
              0);
        }
    });
}
//------------------------------------------------------------------------------
// Model textures
//------------------------------------------------------------------------------
model_viewer_textures::model_viewer_textures(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        int tex_unit = 1;
        for(auto arg : ctx.main_context().args().all_like("--texture")) {
            load(arg, ctx, video, GL.texture_2d_array, tex_unit++);
        }

        load(
          "Checker",
          url{"eagitex:///CheckerTex"},
          ctx,
          video,
          GL.texture_2d_array,
          tex_unit++);
    });
}
//------------------------------------------------------------------------------
auto model_viewer_textures::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

