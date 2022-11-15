/// @example app/029_furry_torus/resources.cpp
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
    info.get_uniform_location("Model") >> model_loc;
    info.get_uniform_location("Camera") >> camera_loc;
}
//------------------------------------------------------------------------------
void surface_program::set_projection(
  video_context& vc,
  orbiting_camera& camera) {
    set(vc, camera_loc, camera.matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
void surface_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& mat) {
    set(vc, model_loc, mat);
}
//------------------------------------------------------------------------------
// hair program
//------------------------------------------------------------------------------
hair_program::hair_program(execution_context& ctx)
  : gl_program_resource{url{"json:///HairProg"}, ctx} {
    loaded.connect(make_callable_ref<&hair_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void hair_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("PrevModel") >> prev_model_loc;
    info.get_uniform_location("CurrModel") >> curr_model_loc;
    info.get_uniform_location("Camera") >> camera_loc;
}
//------------------------------------------------------------------------------
void hair_program::set_projection(video_context& vc, orbiting_camera& camera) {
    set(vc, camera_loc, camera.matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
void hair_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& prev,
  const oglplus::trfmat<4>& curr) {
    set(vc, prev_model_loc, prev);
    set(vc, curr_model_loc, curr);
}
//------------------------------------------------------------------------------
// surface geometry
//------------------------------------------------------------------------------
void shape_surface::init(
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen) {
    gl_geometry_and_bindings::init({gen, vc});

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
// hair geometry
//------------------------------------------------------------------------------
void shape_hair::init(
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen) {

    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(
      glapi,
      shapes::surface_points(
        gen, 256 * 1024, shapes::vertex_attrib_kind::occlusion, vc.parent()));
    gl_geometry_and_bindings::init({shape, vc});

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
shape_texture::shape_texture(url locator, execution_context& ctx)
  : gl_texture_resource{std::move(locator), ctx} {
    loaded.connect(make_callable_ref<&shape_texture::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void shape_texture::_on_loaded(
  const gl_texture_resource::load_info& info) noexcept {
    const auto& GL = info.base.gl_api().constants();
    info.parameter_i(GL.texture_min_filter, GL.linear_mipmap_linear);
    info.parameter_i(GL.texture_mag_filter, GL.linear);
    info.parameter_i(GL.texture_wrap_s, GL.repeat);
    info.parameter_i(GL.texture_wrap_t, GL.repeat);
    info.generate_mipmap();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
