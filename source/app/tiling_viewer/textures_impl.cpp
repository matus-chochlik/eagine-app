/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.tiling_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
static int tex_unit = 0;
//------------------------------------------------------------------------------
// Tiling pattern textures
//------------------------------------------------------------------------------
tiling_viewer_tilings::tiling_viewer_tilings(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        for(auto arg : ctx.main_context().args().all_like("--tiling")) {
            load(
              arg, ctx, video, GL.texture_2d_array, GL.texture0 + tex_unit++);
        }

        const std::array<std::tuple<std::string, url>, 1> args{
          {{"Default", url{"eagitex:///Tlg1024Tex"}}}};

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
auto tiling_viewer_tilings::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
// Transition pattern textures
//------------------------------------------------------------------------------
tiling_viewer_transitions::tiling_viewer_transitions(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        for(auto arg : ctx.main_context().args().all_like("--transition")) {
            load(
              arg, ctx, video, GL.texture_2d_array, GL.texture0 + tex_unit++);
        }

        const std::array<std::tuple<std::string, url>, 2> args{
          {{"PCB", url{"eagitex:///TrnsPCBTex"}},
           {"Checker", url{"eagitex:///TrnsCkrTex"}}}};

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
auto tiling_viewer_transitions::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
// Tile-set image texture
//------------------------------------------------------------------------------
tiling_viewer_tilesets::tiling_viewer_tilesets(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        for(auto arg : ctx.main_context().args().all_like("--tileset")) {
            load(
              arg, ctx, video, GL.texture_2d_array, GL.texture0 + tex_unit++);
        }

        const std::array<std::tuple<std::string, url>, 4> args{
          {{"PCB", url{"eagitex:///TSPCBTex"}},
           {"Connections", url{"eagitex:///TSCon16Tex"}},
           {"Nodes", url{"eagitex:///TSNds16Tex"}},
           {"Blocks", url{"eagitex:///TSBlk16Tex"}}}};

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
auto tiling_viewer_tilesets::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

