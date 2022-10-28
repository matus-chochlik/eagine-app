/// @example app/035_brick_torus/resources.cpp
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
torus_program::torus_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&torus_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void torus_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("LightPosition") >> light_pos_loc;
    info.get_uniform_location("CameraPosition") >> camera_pos_loc;
    info.get_uniform_location("Camera") >> camera_loc;
    info.get_uniform_location("Model") >> model_loc;
    info.get_uniform_location("TextureMap") >> texture_map_loc;
}
//------------------------------------------------------------------------------
void torus_program::set_camera(video_context& video, orbiting_camera& camera) {
    set(video, camera_loc, camera.matrix(video.surface_aspect()));
    set(video, camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
void torus_program::set_model(
  video_context& video,
  const oglplus::trfmat<4>& model) {
    set(video, model_loc, model);
}
//------------------------------------------------------------------------------
void torus_program::set_light(video_context& video, const oglplus::vec3& light) {
    set(video, light_pos_loc, light);
}
//------------------------------------------------------------------------------
void torus_program::set_texture_map(
  video_context& video,
  oglplus::gl_types::int_type unit) {
    set(video, texture_map_loc, unit);
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
torus_geometry::torus_geometry(execution_context& ctx)
  : gl_geometry_and_bindings_resource{
      url{"shape:///unit_torus?"
          "position=true+normal=true+tangent=true+wrap_coord=true+"
          "rings=48+sections=72"},
      ctx} {}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
example_texture::example_texture(
  url locator,
  oglplus::gl_types::int_type tex_unit,
  execution_context& ctx)
  : gl_texture_resource{std::move(locator), ctx.main_video(), ctx.loader()}
  , _tex_unit{tex_unit} {
    loaded.connect(make_callable_ref<&example_texture::_on_loaded>(this));
}
//------------------------------------------------------------------------------
auto example_texture::load_if_needed(execution_context& ctx) noexcept
  -> work_done {
    const auto& GL = ctx.main_video().gl_api().constants();
    return gl_texture_resource::load_if_needed(
      ctx.main_video(),
      ctx.loader(),
      GL.texture_2d_array,
      GL.texture0 + tex_unit());
}
//------------------------------------------------------------------------------
void example_texture::_on_loaded(
  const gl_texture_resource::load_info& info) noexcept {
    const auto& GL = info.base.gl_api().constants();
    info.parameter_i(GL.texture_min_filter, GL.linear_mipmap_linear);
    info.parameter_i(GL.texture_mag_filter, GL.linear);
    info.parameter_i(GL.texture_wrap_s, GL.repeat);
    info.parameter_i(GL.texture_wrap_t, GL.repeat);
    info.generate_mipmap();
}
//------------------------------------------------------------------------------
brick_texture::brick_texture(execution_context& ctx)
  : example_texture{url{"eagitex:///Bricks"}, 0, ctx} {}
//------------------------------------------------------------------------------
stone_texture::stone_texture(execution_context& ctx)
  : example_texture{url{"eagitex:///Stones"}, 1, ctx} {}
//------------------------------------------------------------------------------
} // namespace eagine::app
