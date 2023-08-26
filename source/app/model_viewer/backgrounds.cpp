/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "backgrounds.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_backgrounds::model_viewer_backgrounds(
  execution_context& ctx,
  video_context& video) {
    load(url{"bg:///Icosphere"}, ctx, video);
}
//------------------------------------------------------------------------------
auto model_viewer_backgrounds::clear(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_backgrounds& {
    selected().clear(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

