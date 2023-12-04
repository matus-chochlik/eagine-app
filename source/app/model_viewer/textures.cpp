/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "textures.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_textures::model_viewer_textures(
  execution_context& ctx,
  video_context& video) {
    int tex_unit = 0;
    for(auto arg : ctx.main_context().args().all_like("--texture")) {
        load(arg, ctx, video, tex_unit++);
    }

    load("Checker", url{"eagitex:///CheckerTex"}, ctx, video, tex_unit++);
}
//------------------------------------------------------------------------------
auto model_viewer_textures::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    return current().texture_unit(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app

