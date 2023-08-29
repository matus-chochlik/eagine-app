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
    load("Default", url{"json:///DfaultProg"}, ctx, video);
    load("Normal to Color", url{"json:///Nml2ClrPrg"}, ctx, video);
    load("Tangent to Color", url{"json:///Tgt2ClrPrg"}, ctx, video);
    load("Bi-tangent to Color", url{"json:///Btg2ClrPrg"}, ctx, video);
    load("UV to Color", url{"json:///Wrp2ClrPrg"}, ctx, video);
    load("Edges (normal)", url{"json:///EdgeNProg"}, ctx, video);
    load("Edges (UV)", url{"json:///EdgeUVProg"}, ctx, video);
}
//------------------------------------------------------------------------------
auto model_viewer_programs::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> model_viewer_programs& {
    current().apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::set_camera(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_programs& {
    current().set_camera(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

