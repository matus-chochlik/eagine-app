/// @example app/037_lantern/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// draw program
//------------------------------------------------------------------------------
draw_program::draw_program(video_context& video, resource_loader& loader)
  : gl_program_resource{url{"json:///DrawProg"}, video, loader} {
    loaded.connect(make_callable_ref<&draw_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void draw_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.use_program();
    info.get_uniform_location("Camera") >> _camera_loc;
    info.get_uniform_location("CandleLight") >> _candle_light_loc;
    info.get_uniform_location("AmbientLight") >> _ambient_light_loc;
    info.get_uniform_location("Tex") >> _tex_loc;

    input_bindings = info.base.input_bindings;
}
//------------------------------------------------------------------------------
void draw_program::set_camera(video_context& vc, const orbiting_camera& camera) {
    set(vc, _camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void draw_program::set_candle_light(
  video_context& vc,
  oglplus::gl_types::float_type value) {
    set(vc, _candle_light_loc, value);
}
//------------------------------------------------------------------------------
void draw_program::set_ambient_light(
  video_context& vc,
  oglplus::gl_types::float_type value) {
    set(vc, _ambient_light_loc, value);
}
//------------------------------------------------------------------------------
void draw_program::set_texture_unit(
  video_context& vc,
  oglplus::texture_unit::value_type unit) {
    set(vc, _tex_loc, oglplus::gl_types::int_type(unit));
}
//------------------------------------------------------------------------------
// screen program
//------------------------------------------------------------------------------
screen_program::screen_program(video_context& video, resource_loader& loader)
  : gl_program_resource{url{"json:///ScreenProg"}, video, loader} {
    loaded.connect(make_callable_ref<&screen_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void screen_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.use_program();
    info.get_uniform_location("ScreenSize") >> _screen_size_loc;
    info.get_uniform_location("Tex") >> _tex_loc;

    input_bindings = info.base.input_bindings;
}
//------------------------------------------------------------------------------
void screen_program::set_screen_size(video_context& vc) {
    const auto [w, h] = vc.surface_size();
    set(vc, _screen_size_loc, oglplus::vec2(w, h));
}
//------------------------------------------------------------------------------
void screen_program::set_texture_unit(
  video_context& vc,
  oglplus::texture_unit::value_type unit) {
    set(vc, _tex_loc, oglplus::gl_types::int_type(unit));
}
//------------------------------------------------------------------------------
// pumpkin
//------------------------------------------------------------------------------
pumpkin_geometry::pumpkin_geometry(video_context& video, resource_loader& loader)
  : gl_geometry_and_bindings_resource{url{"json:///Pumpkin"}, video, loader} {
    loaded.connect(make_callable_ref<&pumpkin_geometry::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void pumpkin_geometry::_on_loaded(
  const gl_geometry_and_bindings_resource::load_info& info) noexcept {
    _bounding_sphere = info.base.shape.bounding_sphere();
}
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
pumpkin_texture::pumpkin_texture(video_context& video, resource_loader& loader)
  : gl_texture_resource{url{"eagitex:///PumpkinTex"}, video, loader} {
    loaded.connect(make_callable_ref<&pumpkin_texture::_on_loaded>(this));
}
//------------------------------------------------------------------------------
auto pumpkin_texture::update(
  video_context& video,
  resource_loader& loader) noexcept -> work_done {
    const auto& GL = video.gl_api().constants();
    return gl_texture_resource::update(
      video, loader, GL.texture_2d_array, GL.texture0 + tex_unit());
}
//------------------------------------------------------------------------------
void pumpkin_texture::_on_loaded(
  const gl_texture_resource::load_info& info) noexcept {
    const auto& [gl, GL] = info.base.gl_api();

    gl.active_texture(GL.texture0 + tex_unit());
    gl.bind_texture(GL.texture_2d_array, info.resource);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_t, GL.clamp_to_border);
}
//------------------------------------------------------------------------------
// screen_geometry
//------------------------------------------------------------------------------
screen_geometry::screen_geometry(video_context& video, resource_loader& loader)
  : gl_geometry_and_bindings_resource{
      url{"eagires:///unit_screen?position=true+wrap_coord=true"},
      video,
      loader} {}
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
void draw_buffers::init(video_context& vc) {
    auto& glapi = vc.gl_api();
    base::init(
      vc,
      framebuffer_configuration(vc)
        .add_color_texture(glapi.rgba, glapi.rgba8)
        .add_depth_buffer(),
      view_one(tex_unit()));
}
//------------------------------------------------------------------------------
void draw_buffers::resize(video_context& vc) {
    auto& glapi = vc.gl_api();
    base::resize(
      vc,
      framebuffer_configuration(vc)
        .add_color_texture(glapi.rgba, glapi.rgba8)
        .add_depth_buffer(),
      view_one(tex_unit()));
}
//------------------------------------------------------------------------------
void draw_buffers::draw_off_screen(video_context& vc) {
    base::bind(vc.gl_api());
}
//------------------------------------------------------------------------------
void draw_buffers::draw_on_screen(video_context& vc) {
    base::bind_default(vc.gl_api());
}
//------------------------------------------------------------------------------
} // namespace eagine::app
