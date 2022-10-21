/// @example app/032_translucent_arrow/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
void depth_program::init(video_context& vc) {
    const auto& GL = vc.gl_api();

    create(vc)
      .label(vc, "depth program")
      .add_shader(
        vc,
        GL.vertex_shader,
        oglplus::glsl_string_ref(
          embed<"VSDepth">("vertex_depth.glsl").unpack(vc.parent())),
        "depth vertex shader")
      .link(vc)
      .use(vc)
      .query(vc, "Camera", camera_loc);
}
//------------------------------------------------------------------------------
void depth_program::set_camera(video_context& vc, orbiting_camera& camera) {
    use(vc).set(vc, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void depth_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::init(video_context& vc) {
    const auto& GL = vc.gl_api();

    create(vc)
      .label(vc, "draw program")
      .add_shader(
        vc,
        GL.vertex_shader,
        oglplus::glsl_string_ref(
          embed<"VSDraw">("vertex_draw.glsl").unpack(vc.parent())),
        "draw vertex shader")
      .add_shader(
        vc,
        GL.fragment_shader,
        oglplus::glsl_string_ref(
          embed<"FSDraw">("fragment_draw.glsl").unpack(vc.parent())),
        "draw fragment shader")
      .link(vc)
      .use(vc)
      .query(vc, "Camera", camera_loc)
      .query(vc, "LightPosition", light_pos_loc)
      .query(vc, "DepthTexture", depth_tex_loc);
}
//------------------------------------------------------------------------------
void draw_program::set_depth_texture(
  video_context& vc,
  oglplus::gl_types::int_type tex_unit) {
    use(vc).set(vc, depth_tex_loc, tex_unit);
}
//------------------------------------------------------------------------------
void draw_program::set_camera(video_context& vc, orbiting_camera& camera) {
    use(vc).set(vc, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void draw_program::update(execution_context& ec, video_context& vc) {
    rad += radians_(0.5F * ec.state().frame_duration().value());

    use(vc).set(
      vc,
      light_pos_loc,
      oglplus::vec3(cos(rad) * 5, sin(rad) * 7, sin(rad * 0.618F) * 8));
}
//------------------------------------------------------------------------------
void draw_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Normal");
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

    gl_geometry_and_bindings::init({shape, vc});
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
    vc.gl_api().clean_up(std::move(tex));
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
