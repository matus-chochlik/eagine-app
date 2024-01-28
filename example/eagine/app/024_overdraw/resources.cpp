/// @example app/024_overdraw/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"
#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// draw program
//------------------------------------------------------------------------------
draw_program::draw_program(example& e)
  : gl_program_resource{url{"json:///DrawProg"}, e.context()} {
    loaded.connect(make_callable_ref<&draw_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void draw_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> _camera_loc;
}
//------------------------------------------------------------------------------
void draw_program::set_projection(example& e) {
    e.video().gl_api().set_uniform(
      *this, _camera_loc, e.camera().matrix(e.video()));
}
//------------------------------------------------------------------------------
void draw_program::bind_position_location(
  example& e,
  oglplus::vertex_attrib_location loc) {
    e.video().gl_api().bind_attrib_location(*this, loc, "Position");
}
//------------------------------------------------------------------------------
// screen program
//------------------------------------------------------------------------------
screen_program::screen_program(example& e)
  : gl_program_resource{url{"json:///ScreenProg"}, e.context()} {
    loaded.connect(make_callable_ref<&screen_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void screen_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("ScreenSize") >> _screen_size_loc;
    info.set_uniform("DrawTex", 0);
}
//------------------------------------------------------------------------------
void screen_program::bind_position_location(
  example& e,
  oglplus::vertex_attrib_location loc) {
    e.video().gl_api().bind_attrib_location(*this, loc, "Position");
}
//------------------------------------------------------------------------------
void screen_program::bind_tex_coord_location(
  example& e,
  oglplus::vertex_attrib_location loc) {
    e.video().gl_api().bind_attrib_location(*this, loc, "TexCoord");
}
//------------------------------------------------------------------------------
void screen_program::set_screen_size(example& e) {
    const auto [w, h] = e.video().surface_size();
    e.video().gl_api().set_uniform(
      *this, _screen_size_loc, oglplus::vec2(w, h));
}
//------------------------------------------------------------------------------
// shape geometry
//------------------------------------------------------------------------------
void shape_geometry::init(example& e) {
    const auto& glapi = e.video().gl_api();
    const auto& [gl, GL] = glapi;

    oglplus::shape_generator shape(
      glapi, shapes::unit_cube(shapes::vertex_attrib_kind::position));

    _ops.resize(std_size(shape.operation_count()));
    shape.instructions(glapi, cover(_ops));

    // vao
    gl.gen_vertex_arrays() >> _vao;
    gl.delete_vertex_arrays.later_by(e.cleanup(), _vao);
    gl.bind_vertex_array(_vao);

    // positions
    gl.gen_buffers() >> _positions;
    gl.delete_buffers.later_by(e.cleanup(), _positions);
    shape.attrib_setup(
      glapi,
      _vao,
      _positions,
      position_loc(),
      eagine::shapes::vertex_attrib_kind::position,
      e.context().buffer());

    // indices
    gl.gen_buffers() >> _indices;
    gl.delete_buffers.later_by(e.cleanup(), _indices);
    shape.index_setup(glapi, _indices, e.context().buffer());

    // offsets
    const float d = 1.414F;
    const float h = float(count - 1) * 0.5F;

    std::vector<oglplus::gl_types::float_type> offset_data;
    offset_data.resize(std_size(count * count * count * 4));
    auto p = offset_data.begin();

    for(int k = 0; k != count; ++k) {
        const float z = (float(k) - h) * d;
        for(int j = 0; j != count; ++j) {
            const float y = (float(j) - h) * d;
            for(int i = 0; i != count; ++i) {
                const float x = (float(i) - h) * d;
                *p++ = x;
                *p++ = y;
                *p++ = z;
                *p++ = 0;
            }
        }
    }

    gl.gen_buffers() >> _offsets;
    gl.delete_buffers.later_by(e.cleanup(), _offsets);
    gl.bind_buffer(GL.uniform_buffer, _offsets);
    gl.bind_buffer_base(GL.uniform_buffer, 0, _offsets);
    gl.buffer_data(GL.uniform_buffer, view(offset_data), GL.static_draw);
}
//------------------------------------------------------------------------------
void shape_geometry::draw(example& e) {
    const auto& glapi = e.video().gl_api();
    const auto& gl = glapi;

    gl.bind_vertex_array(_vao);
    draw_instanced_using_instructions(gl, view(_ops), count * count * count);
}
//------------------------------------------------------------------------------
// screen geometry
//------------------------------------------------------------------------------
void screen_geometry::init(example& e) {
    const auto& glapi = e.video().gl_api();
    const auto& [gl, GL] = glapi;

    oglplus::shape_generator shape(
      glapi,
      shapes::unit_screen(
        shapes::vertex_attrib_kind::position |
        shapes::vertex_attrib_kind::wrap_coord));

    _ops.resize(std_size(shape.operation_count()));
    shape.instructions(glapi, cover(_ops));

    // vao
    gl.gen_vertex_arrays() >> _vao;
    gl.delete_vertex_arrays.later_by(e.cleanup(), _vao);
    gl.bind_vertex_array(_vao);

    // positions
    gl.gen_buffers() >> _positions;
    gl.delete_buffers.later_by(e.cleanup(), _positions);
    shape.attrib_setup(
      glapi,
      _vao,
      _positions,
      position_loc(),
      eagine::shapes::vertex_attrib_kind::position,
      e.context().buffer());

    // coords
    gl.gen_buffers() >> _tex_coords;
    gl.delete_buffers.later_by(e.cleanup(), _tex_coords);
    shape.attrib_setup(
      glapi,
      _vao,
      _tex_coords,
      tex_coord_loc(),
      eagine::shapes::vertex_attrib_kind::wrap_coord,
      e.context().buffer());
}
//------------------------------------------------------------------------------
void screen_geometry::draw(example& e) {
    const auto& glapi = e.video().gl_api();
    const auto& gl = glapi;

    gl.bind_vertex_array(_vao);
    draw_using_instructions(gl, view(_ops));
}
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
void draw_buffers::init(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    const auto [width, height] = e.video().surface_size();

    _width = width;
    _height = height;

    gl.gen_textures() >> _tex;
    gl.delete_textures.later_by(e.cleanup(), _tex);
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_rectangle, _tex);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_min_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_mag_filter, GL.nearest);
    gl.tex_image2d(
      GL.texture_rectangle,
      0,
      GL.rg8,
      _width,
      _height,
      0,
      GL.rg,
      GL.unsigned_byte_,
      memory::const_block());

    gl.gen_renderbuffers() >> _rbo;
    gl.delete_renderbuffers.later_by(e.cleanup(), _rbo);
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
}
//------------------------------------------------------------------------------
void draw_buffers::resize(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    const auto [width, height] = e.video().surface_size();

    if(_width != width or _height != height) {
        _width = width;
        _height = height;

        gl.tex_image2d(
          GL.texture_rectangle,
          0,
          GL.rg8,
          _width,
          _height,
          0,
          GL.rg,
          GL.unsigned_byte_,
          memory::const_block());

        gl.renderbuffer_storage(
          GL.renderbuffer, GL.depth_component, _width, _height);
    }
}
//------------------------------------------------------------------------------
void draw_buffers::draw_offscreen(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    gl.bind_framebuffer(GL.draw_framebuffer, _fbo);
}
//------------------------------------------------------------------------------
void draw_buffers::draw_onscreen(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    gl.bind_framebuffer(GL.draw_framebuffer, oglplus::default_framebuffer);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
