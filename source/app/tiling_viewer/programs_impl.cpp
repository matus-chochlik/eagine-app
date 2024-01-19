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
tiling_viewer_programs::tiling_viewer_programs(
  execution_context& ctx,
  video_context& video) {
    const std::array<std::tuple<std::string, url>, 1> args{
      {{"Default", url{"json:///DfaultProg"}}}};

    for(const auto& [name, loc] : args) {
        load(name, loc, ctx, video);
    }
}
//------------------------------------------------------------------------------
auto tiling_viewer_programs::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> tiling_viewer_programs& {
    current().apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_programs::set_camera(
  video_context& video,
  orbiting_camera& camera) -> tiling_viewer_programs& {
    current().set_camera(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_programs::set_tiling_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) -> tiling_viewer_programs& {
    current().set_tiling_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_programs::set_transition_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) -> tiling_viewer_programs& {
    current().set_transition_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_programs::set_tileset_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) -> tiling_viewer_programs& {
    current().set_tileset_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

