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
void torus_program::init(video_context& vc) {
    const auto prog_src{embed<"prog">("brick_torus.oglpprog")};
    create(vc)
      .build(vc, prog_src.unpack(vc.parent()))
      .use(vc)
      .query(vc, "LightPosition", light_pos_loc)
      .query(vc, "CameraPosition", camera_pos_loc)
      .query(vc, "Camera", camera_loc)
      .query(vc, "Model", model_loc)
      .query(vc, "TextureMap", texture_map_loc);
}
//------------------------------------------------------------------------------
void torus_program::set_camera(video_context& vc, orbiting_camera& camera) {
    set(vc, camera_loc, camera.matrix(vc.surface_aspect()))
      .set(vc, camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
void torus_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& model) {
    set(vc, model_loc, model);
}
//------------------------------------------------------------------------------
void torus_program::set_light(video_context& vc, const oglplus::vec3& light) {
    set(vc, light_pos_loc, light);
}
//------------------------------------------------------------------------------
void torus_program::set_texture_map(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    set(vc, texture_map_loc, unit);
}
//------------------------------------------------------------------------------
void torus_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void torus_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Normal");
}
//------------------------------------------------------------------------------
void torus_program::bind_tangent_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Tangent");
}
//------------------------------------------------------------------------------
void torus_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "TexCoord");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void torus_geometry::init(video_context& vc) {
    gl_geometry_and_bindings::init(
      {shapes::unit_torus(
         shapes::vertex_attrib_kind::position |
           shapes::vertex_attrib_kind::normal |
           shapes::vertex_attrib_kind::tangent |
           shapes::vertex_attrib_kind::wrap_coord,
         48,
         72,
         0.5F),
       vc});
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
void torus_textures::init(video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // bricks texture
    const auto brick_tex_src{embed<"BricksTex">("bricks")};

    gl.gen_textures() >> bricks;
    gl.active_texture(GL.texture0 + bricks_map_unit());
    gl.bind_texture(GL.texture_2d_array, bricks);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
    glapi.spec_tex_image3d(
      GL.texture_2d_array,
      0,
      0,
      oglplus::texture_image_block(brick_tex_src.unpack(vc.parent())));
    gl.generate_mipmap(GL.texture_2d_array);

    // stones texture
    const auto stones_tex_src{embed<"StonesTex">("stones")};

    gl.gen_textures() >> stones;
    gl.active_texture(GL.texture0 + stones_map_unit());
    gl.bind_texture(GL.texture_2d_array, stones);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
    glapi.spec_tex_image3d(
      GL.texture_2d_array,
      0,
      0,
      oglplus::texture_image_block(stones_tex_src.unpack(vc.parent())));
    gl.generate_mipmap(GL.texture_2d_array);
}
//------------------------------------------------------------------------------
void torus_textures::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.clean_up(std::move(stones));
    gl.clean_up(std::move(bricks));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
