/// @example application/022_single_pass_edges/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void edges_program::init(video_context& vc) {
    create(vc)
      .build(vc, embed<"prog">("single_pass_edges.oglpprog"))
      .use(vc)
      .query(vc, "Projection", camera_loc)
      .query(vc, "ViewportDimensions", vp_dim_loc);
}
//------------------------------------------------------------------------------
void edges_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto [width, height] = vc.surface_size();
    set(vc, camera_loc, camera.matrix(vc));
    set(vc, vp_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
void edges_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(video_context& vc) {
    gl_geometry_and_bindings::init(
      {shapes::center(eagine::shapes::ortho_array_xyz(
         shapes::scale(
           shapes::unit_icosahedron(shapes::vertex_attrib_kind::position),
           {0.5F, 0.5F, 0.5F}),
         {1.F, 1.F, 1.F},
         {3, 3, 3})),
       vc});
}
//------------------------------------------------------------------------------
} // namespace eagine::app
