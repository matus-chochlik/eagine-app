/// @example app/026_halo/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// surface program
//------------------------------------------------------------------------------
surface_program::surface_program(execution_context& ctx)
  : gl_program_resource{url{"json:///SurfProg"}, ctx} {
    loaded.connect(make_callable_ref<&surface_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void surface_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Model") >> _model_loc;
    info.get_uniform_location("View") >> _view_loc;
    info.get_uniform_location("Projection") >> _projection_loc;
}
//------------------------------------------------------------------------------
void surface_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto& gl = vc.gl_api();
    gl.use_program(*this);
    gl.set_uniform(
      *this, _model_loc, oglplus::matrix_rotation_x(right_angles_(t))());
    gl.set_uniform(*this, _view_loc, camera.transform_matrix());
    gl.set_uniform(
      *this, _projection_loc, camera.perspective_matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
// halo program
//------------------------------------------------------------------------------
halo_program::halo_program(execution_context& ctx)
  : gl_program_resource{url{"json:///HaloProg"}, ctx} {
    loaded.connect(make_callable_ref<&halo_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void halo_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Model") >> _model_loc;
    info.get_uniform_location("View") >> _view_loc;
    info.get_uniform_location("Projection") >> _projection_loc;
    info.get_uniform_location("CameraPos") >> _camera_pos_loc;
}
//------------------------------------------------------------------------------
void halo_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto& gl = vc.gl_api();
    gl.use_program(*this);

    gl.set_uniform(
      *this, _model_loc, oglplus::matrix_rotation_x(right_angles_(t))());
    gl.set_uniform(*this, _view_loc, camera.transform_matrix());
    gl.set_uniform(
      *this, _projection_loc, camera.perspective_matrix(vc.surface_aspect()));
    gl.set_uniform(*this, _camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
shape_geometry::shape_geometry(execution_context& ctx)
  : gl_geometry_and_bindings_resource{url{"json:///TwstdSphre"}, ctx} {}
//------------------------------------------------------------------------------
} // namespace eagine::app
