/// @example app/021_cel_shading/resources.cpp
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
cel_program::cel_program(video_context& video, resource_loader& loader)
  : gl_program_resource{url{"json:///CelProg"}, video, loader} {
    loaded.connect(make_callable_ref<&cel_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void cel_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.use_program();
    info.get_uniform_location("Modelview") >> modelview_loc;
    info.get_uniform_location("Projection") >> projection_loc;
}
//------------------------------------------------------------------------------
void cel_program::set_projection(video_context& video, orbiting_camera& camera) {
    if(camera.has_changed()) {
        set(video, projection_loc, camera.matrix(video));
    }
}
//------------------------------------------------------------------------------
void cel_program::set_modelview(execution_context& ec, video_context& video) {
    shp_turns += 0.1F * ec.state().frame_duration().value();

    set(
      video,
      modelview_loc,
      oglplus::matrix_rotation_x(turns_(shp_turns) / 1) *
        oglplus::matrix_rotation_y(turns_(shp_turns) / 2) *
        oglplus::matrix_rotation_z(turns_(shp_turns) / 3));
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
icosahedron_geometry::icosahedron_geometry(
  video_context& video,
  resource_loader& loader)
  : gl_geometry_and_bindings_resource{
      url{"shape:///unit_icosahedron?position=true"},
      video,
      loader} {}
//------------------------------------------------------------------------------
} // namespace eagine::app
