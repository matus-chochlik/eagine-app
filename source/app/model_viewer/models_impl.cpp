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
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_models::model_viewer_models(
  execution_context& ctx,
  video_context& video) {
    for(auto arg : ctx.main_context().args().all_like("--model")) {
        load(arg, ctx, video);
    }

    load(
      "Screen",
      url{"shape:///unit_screen"
          "?position=true&normal=true&tangent=true&bitangent=true"
          "&wrap_coord=true"},
      ctx,
      video);
    load(
      "Twisted torus",
      url{"shape:///unit_twisted_torus"
          "?position=true&normal=true&wrap_coord=true"
          "&rings=96"},
      ctx,
      video);
    load(
      "Torus",
      url{"shape:///unit_torus"
          "?position=true&normal=true&tangent=true&bitangent=true"
          "&wrap_coord=true&roughness=true&occlusion=true"
          "&rings=36&sections=72"},
      ctx,
      video);
    load(
      "Icosahedron",
      url{"shape:///unit_icosahedron?position=true&normal=true"},
      ctx,
      video);
    load(
      "Spikosahedron",
      url{"shape:///model_spikosahedron"
          "?position=true&normal=true&tangent=true&bitangent=true"
          "&wrap_coord=true&color=true&roughness=true&occlusion=true"},
      ctx,
      video);
    load(
      "Round cube",
      url{"shape:///unit_round_cube"
          "?position=true&normal=true&tangent=true&bitangent=true"
          "&wrap_coord=true"},
      ctx,
      video);
    load(
      "Cube",
      url{"shape:///model_cube"
          "?position=true&normal=true&tangent=true&bitangent=true"
          "&wrap_coord=true&color=true&occlusion=true"},
      ctx,
      video);
    load("Arrow", url{"json:///Arrow"}, ctx, video);
}
//------------------------------------------------------------------------------
auto model_viewer_models::bounding_sphere() noexcept -> oglplus::sphere {
    return current().bounding_sphere();
}
//------------------------------------------------------------------------------
auto model_viewer_models::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    return current().attrib_bindings();
}
//------------------------------------------------------------------------------
auto model_viewer_models::draw(video_context& video) -> model_viewer_models& {
    video.with_gl([](auto& gl, auto& GL) { gl.disable(GL.cull_face); });
    current().draw(video);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

