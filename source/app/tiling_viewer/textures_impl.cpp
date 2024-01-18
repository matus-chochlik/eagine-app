/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
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
// Tiling pattern textures
//------------------------------------------------------------------------------
tiling_viewer_tilings::tiling_viewer_tilings(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        if(auto arg{ctx.main_context().args().find("--tiling")}) {
            load(arg, ctx, video, GL.texture_2d_array, 0);
        } else {
            load(
              "Default",
              url{"eagitex:///Tlg1024Tex"},
              ctx,
              video,
              GL.texture_2d_array,
              0);
        }
    });
}
//------------------------------------------------------------------------------
auto tiling_viewer_tilings::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
// Transition pattern textures
//------------------------------------------------------------------------------
tiling_viewer_transitions::tiling_viewer_transitions(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        if(auto arg{ctx.main_context().args().find("--transition")}) {
            load(arg, ctx, video, GL.texture_2d_array, 1);
        } else {
            load(
              "PCB",
              url{"eagitex:///TrnsPCBTex"},
              ctx,
              video,
              GL.texture_2d_array,
              1);
        }
    });
}
//------------------------------------------------------------------------------
auto tiling_viewer_transitions::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
// Tile-set image texture
//------------------------------------------------------------------------------
tiling_viewer_tilesets::tiling_viewer_tilesets(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        int tex_unit = 2;
        for(auto arg : ctx.main_context().args().all_like("--tileset")) {
            load(arg, ctx, video, GL.texture_2d_array, tex_unit++);
        }

        load(
          "Connections",
          url{"eagitex:///TSCon16Tex"},
          ctx,
          video,
          GL.texture_2d_array,
          tex_unit++);
        load(
          "Nodes",
          url{"eagitex:///TSNds16Tex"},
          ctx,
          video,
          GL.texture_2d_array,
          tex_unit++);
        load(
          "Blocks",
          url{"eagitex:///TSBlk16Tex"},
          ctx,
          video,
          GL.texture_2d_array,
          tex_unit++);
    });
}
//------------------------------------------------------------------------------
auto tiling_viewer_tilesets::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

