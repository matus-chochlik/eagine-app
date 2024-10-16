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
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_backgrounds::model_viewer_backgrounds(
  execution_context& ctx,
  video_context& video) {
    load("Skybox", url{"eagibg:///Skybox"}, ctx, video);
    load("Icosphere", url{"eagibg:///Icosphere"}, ctx, video);
}
//------------------------------------------------------------------------------
auto model_viewer_backgrounds::set_skybox_unit(
  video_context& video,
  oglplus::texture_unit tu) -> model_viewer_backgrounds& {
    current().set_skybox_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_backgrounds::clear(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_backgrounds& {
    current().use(video);
    current().clear(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_backgrounds::clear_default(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_backgrounds& {
    at(1).use(video);
    at(1).clear(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

