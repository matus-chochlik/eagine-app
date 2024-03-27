/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
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
static int tex_unit = 0;
//------------------------------------------------------------------------------
// Cube maps
//------------------------------------------------------------------------------
model_viewer_cube_maps::model_viewer_cube_maps(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        for(auto arg : ctx.main_context().args().all_like("--cube-map")) {
            load(
              arg, ctx, video, GL.texture_cube_map, GL.texture0 + tex_unit++);
        }

        if(ctx.main_context().args().find("--noise-cube")) {
            load(
              "Noise",
              url{"eagitex:///NoiseCube"},
              ctx,
              video,
              GL.texture_cube_map,
              GL.texture0 + tex_unit++);
        }

        const std::array<std::tuple<std::string, url>, 1> args{
          {{"Checker", url{"eagitex:///CheckerCub"}}}};

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
auto model_viewer_cube_maps::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
// Model textures
//------------------------------------------------------------------------------
model_viewer_textures::model_viewer_textures(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        for(auto arg : ctx.main_context().args().all_like("--texture")) {
            load(
              arg, ctx, video, GL.texture_2d_array, GL.texture0 + tex_unit++);
        }

        const std::array<std::tuple<std::string, url>, 1> args{
          {{"Checker", url{"eagitex:///CheckerTex"}}}};

        for(const auto& [name, loc] : args) {
            load(
              name,
              loc,
              ctx,
              video,
              GL.texture_2d_array,
              GL.texture0 + tex_unit++);
        }
    });
}
//------------------------------------------------------------------------------
auto model_viewer_textures::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

