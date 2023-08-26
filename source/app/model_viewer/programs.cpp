/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "programs.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_programs::model_viewer_programs(
  execution_context& ctx,
  video_context& video) {
    load(url{"json:///DfaultProg"}, ctx, video);
    load(url{"json:///Nml2ClrPrg"}, ctx, video);
}
//------------------------------------------------------------------------------
auto model_viewer_programs::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> model_viewer_programs& {
    selected().apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::set_camera(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_programs& {
    selected().set_camera(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

