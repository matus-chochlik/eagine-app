/// @example app/032_translucent_arrow/resources.cpp
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

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
void depth_program::init(execution_context& ec, video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();

    // vertex shader
    oglplus::owned_shader_name vs;
    auto vs_src = embed(EAGINE_ID(VSDepth), "vertex_depth.glsl");
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.object_label(vs, "depth vertex shader");
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_src.unpack(ec)));
    gl.compile_shader(vs);

    // program
    gl.create_program() >> prog;
    gl.object_label(prog, "depth program");
    gl.attach_shader(prog, vs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Camera") >> camera_loc;
}
//------------------------------------------------------------------------------
void depth_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void depth_program::set_camera(video_context& vc, orbiting_camera& camera) {
    const auto& gl = vc.gl_api();

    gl.use_program(prog);
    gl.set_uniform(prog, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void depth_program::update(video_context& vc) {
    vc.gl_api().use_program(prog);
}
//------------------------------------------------------------------------------
void depth_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    vc.gl_api().bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::init(execution_context& ec, video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();

    // vertex shader
    oglplus::owned_shader_name vs;
    auto vs_src = embed(EAGINE_ID(VSDraw), "vertex_draw.glsl");
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.object_label(vs, "draw vertex shader");
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_src.unpack(ec)));
    gl.compile_shader(vs);

    // fragment shader
    oglplus::owned_shader_name fs;
    auto fs_src = embed(EAGINE_ID(FSDraw), "fragment_draw.glsl");
    gl.create_shader(GL.fragment_shader) >> fs;
    auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.object_label(fs, "draw fragment shader");
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_src.unpack(ec)));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.object_label(prog, "draw program");
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "LightPosition") >> light_pos_loc;
    gl.get_uniform_location(prog, "DepthTexture") >> depth_tex_loc;
}
//------------------------------------------------------------------------------
void draw_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void draw_program::set_depth_texture(
  video_context& vc,
  oglplus::gl_types::int_type tex_unit) {
    const auto& gl = vc.gl_api();

    gl.use_program(prog);
    gl.set_uniform(prog, depth_tex_loc, tex_unit);
}
//------------------------------------------------------------------------------
void draw_program::set_camera(video_context& vc, orbiting_camera& camera) {
    const auto& gl = vc.gl_api();

    gl.use_program(prog);
    gl.set_uniform(prog, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void draw_program::update(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    rad += radians_(0.5F * ec.state().frame_duration().value());

    gl.use_program(prog);
    gl.set_uniform(
      prog,
      light_pos_loc,
      oglplus::vec3(cos(rad) * 5, sin(rad) * 7, sin(rad * 0.618F) * 8));
}
//------------------------------------------------------------------------------
void draw_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    vc.gl_api().bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    vc.gl_api().bind_attrib_location(prog, loc, "Normal");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void shape_geometry::init(
  const std::shared_ptr<shapes::generator>& gen,
  video_context& vc) {
    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(glapi, gen);
    bound_sphere = shape.bounding_sphere();

    geometry_and_bindings::init(glapi, shape, vc.parent().buffer());
}
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
void depth_texture::init(execution_context&, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.gen_textures() >> tex;
}
//------------------------------------------------------------------------------
void depth_texture::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_textures(std::move(tex));
}
//------------------------------------------------------------------------------
void depth_texture::reshape(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    const auto [width, height] = vc.surface_size();

    gl.active_texture(GL.texture0 + tex_unit);
    gl.bind_texture(GL.texture_rectangle, tex);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_min_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_mag_filter, GL.nearest);
    gl.tex_image2d(
      GL.texture_rectangle,
      0,
      GL.depth_component,
      width,
      height,
      0,
      GL.depth_component,
      GL.float_,
      memory::const_block());

    gl.viewport(width, height);
    gl.clear(GL.depth_buffer_bit);
}
//------------------------------------------------------------------------------
void depth_texture::copy_from_fb(video_context& vc) {
    const auto [width, height] = vc.surface_size();
    const auto& [gl, GL] = vc.gl_api();

    gl.copy_tex_image2d(
      GL.texture_rectangle, 0, GL.depth_component, 0, 0, width, height, 0);
}
//------------------------------------------------------------------------------

} // namespace eagine::app
