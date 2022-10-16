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
void draw_program::init(video_context& vc) {
    // vertex shader
    const auto vs_source = search_resource("DrawVert");
    const auto fs_source = search_resource("DrawFrag");

    const auto& GL = vc.gl_api();

    create(vc)
      .add_shader(vc, GL.vertex_shader, vs_source)
      .add_shader(vc, GL.fragment_shader, fs_source)
      .link(vc)
      .use(vc)
      .query(vc, "Camera", _camera_loc)
      .query(vc, "CandleLight", _candle_light_loc)
      .query(vc, "AmbientLight", _ambient_light_loc)
      .query(vc, "Tex", _tex_loc)
      .clean_up_later(vc);
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
  oglplus::gl_types::int_type unit) {
    set(vc, _tex_loc, unit);
}
//------------------------------------------------------------------------------
void draw_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location position_loc) {
    bind(vc, position_loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location normal_loc) {
    bind(vc, normal_loc, "Normal");
}
//------------------------------------------------------------------------------
void draw_program::bind_wrap_coord_location(
  video_context& vc,
  oglplus::vertex_attrib_location coord_loc) {
    bind(vc, coord_loc, "WrapCoord");
}
//------------------------------------------------------------------------------
// screen program
//------------------------------------------------------------------------------
void screen_program::init(video_context& vc) {
    // vertex shader
    const auto vs_source = search_resource("ScreenVert");
    const auto fs_source = search_resource("ScreenFrag");

    const auto& GL = vc.gl_api();

    create(vc)
      .add_shader(
        vc,
        GL.vertex_shader,
        oglplus::glsl_string_ref(vs_source.unpack(vc.parent())))
      .add_shader(
        vc,
        GL.fragment_shader,
        oglplus::glsl_string_ref(fs_source.unpack(vc.parent())))
      .link(vc)
      .use(vc)
      .query(vc, "ScreenSize", _screen_size_loc)
      .query(vc, "Tex", _tex_loc)
      .clean_up_later(vc);
}
//------------------------------------------------------------------------------
void screen_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void screen_program::bind_coord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Coord");
}
//------------------------------------------------------------------------------
void screen_program::set_screen_size(video_context& vc) {
    const auto [w, h] = vc.surface_size();
    set(vc, _screen_size_loc, oglplus::vec2(w, h));
}
//------------------------------------------------------------------------------
void screen_program::set_texture_unit(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    set(vc, _tex_loc, unit);
}
//------------------------------------------------------------------------------
// pumpkin
//------------------------------------------------------------------------------
pumpkin_model::pumpkin_model(video_context& video, resource_loader& loader)
  : _geom{url{"json:///Pumpkin"}, video, loader} {
    _geom.loaded.connect(
      make_callable_ref<&pumpkin_model::_on_geom_loaded>(this));

    const auto& glapi = video.gl_api();
    const auto& [gl, GL] = glapi;

    // textures
    const auto tex_src{search_resource("PumpkinTex")};

    gl.gen_textures() >> _tex;
    gl.active_texture(GL.texture0 + tex_unit());
    gl.bind_texture(GL.texture_2d_array, _tex);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image3d(
      GL.texture_2d_array,
      0,
      0,
      oglplus::texture_image_block(tex_src.unpack(video.parent())));
}
//------------------------------------------------------------------------------
void pumpkin_model::update(
  video_context& video,
  resource_loader& loader) noexcept {
    _geom.update(video, loader);
}
//------------------------------------------------------------------------------
void pumpkin_model::clean_up(
  video_context& video,
  resource_loader& loader) noexcept {
    const auto& gl = video.gl_api();

    gl.clean_up(std::move(_tex));
    _geom.clean_up(video, loader);
}
//------------------------------------------------------------------------------
// screen
//------------------------------------------------------------------------------
void screen_geometry::init(video_context& vc) {
    geometry_and_bindings::init(
      {shapes::unit_screen(
         shapes::vertex_attrib_kind::position |
         shapes::vertex_attrib_kind::wrap_coord),
       vc});

    vc.clean_up_later(*this);
}
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
