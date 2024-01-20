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
model_viewer_programs::model_viewer_programs(
  execution_context& ctx,
  video_context& video) {
    for(auto arg : ctx.main_context().args().all_like("--program")) {
        load(arg, ctx, video);
    }

    const std::array<std::tuple<std::string, url>, 7> args{
      {{"Default", url{"json:///DfaultProg"}},
       {"Normal to Color", url{"json:///Nml2ClrPrg"}},
       {"Tangent to Color", url{"json:///Tgt2ClrPrg"}},
       {"Bi-tangent to Color", url{"json:///Btg2ClrPrg"}},
       {"UV to Color", url{"json:///Wrp2ClrPrg"}},
       {"Edges (UV)", url{"json:///EdgeUVProg"}},
       {"Metal", url{"json:///MetalProg"}}}};

    for(const auto& [name, loc] : args) {
        load(name, loc, ctx, video);
    }
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
auto model_viewer_programs::set_cube_map_unit(
  video_context& video,
  oglplus::texture_unit tu) -> model_viewer_programs& {
    current().set_cube_map_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::set_texture_unit(
  video_context& video,
  oglplus::texture_unit tu) -> model_viewer_programs& {
    current().set_texture_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

