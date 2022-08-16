/// @example app/021_cel_shading/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

#include <eagine/app/camera.hpp>
#include <eagine/app/context.hpp>
#include <eagine/embed.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/icosahedron.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void cel_program::init(video_context& vc) {
    create(vc)
      .build(vc, embed<"prog">("cel_shading.oglpprog"))
      .use(vc)
      .query(vc, "Projection", projection_loc)
      .query(vc, "Modelview", modelview_loc);
}
//------------------------------------------------------------------------------
void cel_program::set_projection(video_context& vc, orbiting_camera& camera) {
    if(camera.has_changed()) {
        set(vc, projection_loc, camera.matrix(vc));
    }
}
//------------------------------------------------------------------------------
void cel_program::set_modelview(execution_context& ec, video_context& vc) {
    shp_turns += 0.1F * ec.state().frame_duration().value();

    set(
      vc,
      modelview_loc,
      oglplus::matrix_rotation_x(turns_(shp_turns) / 1) *
        oglplus::matrix_rotation_y(turns_(shp_turns) / 2) *
        oglplus::matrix_rotation_z(turns_(shp_turns) / 3));
}
//------------------------------------------------------------------------------
void cel_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(video_context& vc) {
    geometry_and_bindings::init(
      shapes::unit_icosahedron(shapes::vertex_attrib_kind::position), vc);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
