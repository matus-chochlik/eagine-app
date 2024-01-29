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
tiling_viewer_models::tiling_viewer_models(
  execution_context& ctx,
  video_context& video) {
    load(
      "Round cube",
      url{"shape:///unit_round_cube"
          "?position=true+normal=true+tangent=true+bitangent=true"
          "+wrap_coord=true+divisions=16"},
      ctx,
      video);
    load(
      "Screen",
      url{"shape:///unit_screen"
          "?position=true+normal=true+tangent=true+bitangent=true"
          "+wrap_coord=true"},
      ctx,
      video);
}
//------------------------------------------------------------------------------
auto tiling_viewer_models::bounding_sphere() noexcept -> oglplus::sphere {
    return current().bounding_sphere();
}
//------------------------------------------------------------------------------
auto tiling_viewer_models::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    return current().attrib_bindings();
}
//------------------------------------------------------------------------------
auto tiling_viewer_models::draw(video_context& video) -> tiling_viewer_models& {
    video.with_gl([](auto& gl, auto& GL) { gl.disable(GL.cull_face); });
    current().draw(video);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

