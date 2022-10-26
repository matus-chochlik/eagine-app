/// @example application/025_recursive_cube/resources.cpp
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
cube_program::cube_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&cube_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void cube_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Projection") >> projection_loc;
    info.get_uniform_location("Modelview") >> modelview_loc;
    info.get_uniform_location("LightPos") >> light_pos_loc;
    info.get_uniform_location("CubeTex") >> cube_tex_loc;

    input_bindings = info.base.input_bindings;
}
//------------------------------------------------------------------------------
void cube_program::set_texture(
  execution_context& ec,
  oglplus::gl_types::int_type tex_unit) {
    set(ec.main_video(), cube_tex_loc, tex_unit);
}
//------------------------------------------------------------------------------
void cube_program::set_projection(
  execution_context& ec,
  const oglplus::trfmat<4>& proj_mat) {
    set(ec.main_video(), projection_loc, proj_mat);
}
//------------------------------------------------------------------------------
void cube_program::prepare_frame(execution_context& ec) {
    rad += radians_(0.5F * ec.state().frame_duration().value());
    set(
      ec.main_video(),
      modelview_loc,
      oglplus::matrix_rotation_x(rad * 1.F) *
        oglplus::matrix_rotation_y(rad * 2.F) *
        oglplus::matrix_rotation_z(rad * 3.F));
    set(
      ec.main_video(),
      light_pos_loc,
      oglplus::vec3(cos(rad) * 4, sin(rad) * 4, 8));
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
cube_geometry::cube_geometry(execution_context& ec)
  : gl_geometry_and_bindings_resource{
      url{"shape:///unit_cube?position=true+normal=true+face_coord=true"},
      ec} {}
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
void cube_draw_buffers::init(execution_context& ec) {
    const auto& glapi = ec.main_video().gl_api();
    const auto& [gl, GL] = glapi;

    for(int i = 0; i < 2; ++i) {
        auto& obj = objs.front();
        obj.tex_unit = i;
        gl.gen_textures() >> obj.tex;
        gl.active_texture(GL.texture0 + i);
        gl.bind_texture(GL.texture_2d, obj.tex);
        gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.nearest);
        gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.nearest);
        gl.tex_image2d(
          GL.texture_2d,
          0,
          GL.rgb,
          tex_side,
          tex_side,
          0,
          GL.rgb,
          GL.unsigned_byte_,
          memory::const_block());

        gl.gen_renderbuffers() >> obj.rbo;
        gl.bind_renderbuffer(GL.renderbuffer, obj.rbo);
        gl.renderbuffer_storage(
          GL.renderbuffer, GL.depth_component, tex_side, tex_side);

        gl.gen_framebuffers() >> obj.fbo;
        gl.bind_framebuffer(GL.draw_framebuffer, obj.fbo);

        gl.framebuffer_texture2d(
          GL.draw_framebuffer, GL.color_attachment0, GL.texture_2d, obj.tex, 0);
        gl.framebuffer_renderbuffer(
          GL.draw_framebuffer, GL.depth_attachment, GL.renderbuffer, obj.rbo);

        gl.viewport(tex_side, tex_side);
        gl.clear(GL.color_buffer_bit);

        objs.swap();
    }

    gl.bind_framebuffer(GL.draw_framebuffer, oglplus::default_framebuffer);
    gl.bind_renderbuffer(GL.renderbuffer, oglplus::no_renderbuffer);
}
//------------------------------------------------------------------------------
void cube_draw_buffers::clean_up(execution_context& ec) {
    const auto& gl = ec.main_video().gl_api();
    for(int i = 0; i < 2; ++i) {
        auto& obj = objs.front();
        gl.delete_renderbuffers(std::move(obj.rbo));
        gl.delete_textures(std::move(obj.tex));
        gl.delete_framebuffers(std::move(obj.fbo));
        objs.swap();
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app
