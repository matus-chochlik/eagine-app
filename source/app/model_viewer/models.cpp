/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "models.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_models::model_viewer_models(
  execution_context& ctx,
  video_context& video) {
    load("Traffic cone", url{"json:///TraficCone"}, ctx, video);
    load("Guitar", url{"json:///Guitar"}, ctx, video);
    load("Stool", url{"json:///Stool"}, ctx, video);
    load(
      "Twisted torus",
      url{"shape:///unit_twisted_torus?position=true+normal=true"},
      ctx,
      video);
    load(
      "Torus",
      url{"shape:///unit_torus"
          "?position=true+normal=true+tangent=true+bitangent=true"},
      ctx,
      video);
    load(
      "Round cube",
      url{"shape:///unit_round_cube"
          "?position=true+normal=true+tangent=true+bitangent=true"},
      ctx,
      video);
    load(
      "Icosahedron",
      url{"shape:///unit_icosahedron?position=true+normal=true"},
      ctx,
      video);
}
//------------------------------------------------------------------------------
auto model_viewer_models::bounding_sphere() noexcept -> oglplus::sphere {
    return selected().bounding_sphere();
}
//------------------------------------------------------------------------------
auto model_viewer_models::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    return selected().attrib_bindings();
}
//------------------------------------------------------------------------------
auto model_viewer_models::draw(video_context& video) -> model_viewer_models& {
    selected().draw(video);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

