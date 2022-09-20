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
void surface_program::init(video_context& vc) {
    const auto prog_src{embed<"SurfProg">("furry_torus_surface.oglpprog")};
    create(vc)
      .label(vc, "surface program")
      .build(vc, prog_src.unpack(vc.parent()))
      .use(vc)
      .query(vc, "Model", model_loc)
      .query(vc, "Camera", camera_loc)
      .query(vc, "Tex", texture_loc)
      .clean_up_later(vc);
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
void surface_program::set_texture(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    set(vc, texture_loc, unit);
}
//------------------------------------------------------------------------------
void surface_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void surface_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "TexCoord");
}
//------------------------------------------------------------------------------
void surface_program::bind_occlusion_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Occlusion");
}
//------------------------------------------------------------------------------
// hair program
//------------------------------------------------------------------------------
void hair_program::init(video_context& vc) {
    const auto prog_src{embed<"HairProg">("furry_torus_hair.oglpprog")};
    create(vc)
      .label(vc, "hair program")
      .build(vc, prog_src.unpack(vc.parent()))
      .use(vc)
      .query(vc, "PrevModel", prev_model_loc)
      .query(vc, "CurrModel", curr_model_loc)
      .query(vc, "Camera", camera_loc)
      .query(vc, "Tex", texture_loc)
      .clean_up_later(vc);
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
void hair_program::set_texture(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    set(vc, texture_loc, unit);
}
//------------------------------------------------------------------------------
void hair_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void hair_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Normal");
}
//------------------------------------------------------------------------------
void hair_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "TexCoord");
}
//------------------------------------------------------------------------------
void hair_program::bind_occlusion_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Occlusion");
}
//------------------------------------------------------------------------------
// surface geometry
//------------------------------------------------------------------------------
void shape_surface::init(
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen) {
    geometry_and_bindings::init(gen, vc);

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
    geometry_and_bindings::init(shape, vc);

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
void shape_textures::init(video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // zebra texture
    const auto zebra_tex_src{embed<"ZebraTex">("zebra_fur")};

    gl.gen_textures() >> zebra;
    gl.active_texture(GL.texture0 + map_unit_zebra());
    gl.bind_texture(GL.texture_2d, zebra);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_swizzle_g, GL.red);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_swizzle_b, GL.red);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(zebra_tex_src.unpack(vc.parent())));
    gl.generate_mipmap(GL.texture_2d);

    // monkey texture
    const auto monkey_tex_src{embed<"MonkeyTex">("monkey")};

    gl.gen_textures() >> monkey;
    gl.active_texture(GL.texture0 + map_unit_monkey());
    gl.bind_texture(GL.texture_2d, monkey);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_edge);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_edge);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(monkey_tex_src.unpack(vc.parent())));
    gl.generate_mipmap(GL.texture_2d);

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
void shape_textures::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.clean_up(std::move(monkey));
    gl.clean_up(std::move(zebra));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
