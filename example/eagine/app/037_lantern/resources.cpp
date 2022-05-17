/// @example app/037_lantern/resources.cpp
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
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/screen.hpp>
#include <eagine/shapes/value_tree.hpp>
#include <eagine/value_tree/json.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// draw program
//------------------------------------------------------------------------------
void draw_program::init(video_context& vc) {
    // vertex shader
    auto vs_source = embed(EAGINE_ID(DrawVert), "draw_vertex.glsl");
    auto fs_source = embed(EAGINE_ID(DrawFrag), "draw_fragment.glsl");

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
    auto vs_source = embed(EAGINE_ID(ScreenVert), "screen_vertex.glsl");
    auto fs_source = embed(EAGINE_ID(ScreenFrag), "screen_fragment.glsl");

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
void pumpkin_geometry::init(video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    const auto json_text =
      as_chars(embed(EAGINE_ID(ShapeJson), "pumpkin.json").unpack(vc.parent()));

    oglplus::shape_generator shape(
      glapi,
      shapes::from_value_tree(
        valtree::from_json_text(json_text, vc.parent()), vc.parent()));
    _bounding_sphere = shape.bounding_sphere();

    // geometry
    geometry_and_bindings::init(shape, vc);

    // textures
    const auto tex_src{embed(EAGINE_ID(PumpkinTex), "pumpkin")};

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
      oglplus::texture_image_block(tex_src.unpack(vc.parent())));

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
void pumpkin_geometry::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.delete_textures(std::move(_tex));
    geometry_and_bindings::clean_up(vc);
}
//------------------------------------------------------------------------------
// screen
//------------------------------------------------------------------------------
void screen_geometry::init(video_context& vc) {
    geometry_and_bindings::init(
      shapes::unit_screen(
        shapes::vertex_attrib_kind::position |
        shapes::vertex_attrib_kind::wrap_coord),
      vc);

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
void draw_buffers::init(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    const auto [width, height] = vc.surface_size();

    _width = width;
    _height = height;

    gl.gen_textures() >> _tex;
    gl.active_texture(GL.texture0 + tex_unit());
    gl.bind_texture(GL.texture_rectangle, _tex);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_min_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_mag_filter, GL.nearest);
    gl.tex_image2d(
      GL.texture_rectangle,
      0,
      GL.rgba8,
      _width,
      _height,
      0,
      GL.rgba,
      GL.unsigned_byte_,
      memory::const_block());

    gl.gen_renderbuffers() >> _rbo;
    gl.bind_renderbuffer(GL.renderbuffer, _rbo);
    gl.renderbuffer_storage(
      GL.renderbuffer, GL.depth_component, _width, _height);

    gl.gen_framebuffers() >> _fbo;
    gl.bind_framebuffer(GL.draw_framebuffer, _fbo);

    gl.framebuffer_texture2d(
      GL.draw_framebuffer, GL.color_attachment0, GL.texture_rectangle, _tex, 0);
    gl.framebuffer_renderbuffer(
      GL.draw_framebuffer, GL.depth_attachment, GL.renderbuffer, _rbo);

    gl.bind_framebuffer(GL.draw_framebuffer, oglplus::default_framebuffer);

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
void draw_buffers::resize(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    const auto [width, height] = vc.surface_size();

    if(_width != width || _height != height) {
        _width = width;
        _height = height;

        gl.tex_image2d(
          GL.texture_rectangle,
          0,
          GL.rgba8,
          _width,
          _height,
          0,
          GL.rgba,
          GL.unsigned_byte_,
          memory::const_block());

        gl.renderbuffer_storage(
          GL.renderbuffer, GL.depth_component, _width, _height);
    }
}
//------------------------------------------------------------------------------
void draw_buffers::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_renderbuffers(std::move(_rbo));
    gl.delete_textures(std::move(_tex));
}
//------------------------------------------------------------------------------
void draw_buffers::draw_off_screen(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    gl.bind_framebuffer(GL.draw_framebuffer, _fbo);
}
//------------------------------------------------------------------------------
void draw_buffers::draw_on_screen(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    gl.bind_framebuffer(GL.draw_framebuffer, oglplus::default_framebuffer);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
