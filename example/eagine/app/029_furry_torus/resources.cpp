/// @example app/029_furry_torus/resources.cpp
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
#include <eagine/shapes/surface_points.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// surface program
//------------------------------------------------------------------------------
void surface_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> prog;
    gl.object_label(prog, "surface program");

    const auto prog_src{
      embed(EAGINE_ID(SurfProg), "furry_torus_surface.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Model") >> model_loc;
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "Tex") >> texture_loc;
}
//------------------------------------------------------------------------------
void surface_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void surface_program::use(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.use_program(prog);
}
//------------------------------------------------------------------------------
void surface_program::set_projection(
  video_context& vc,
  orbiting_camera& camera) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, camera_loc, camera.matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
void surface_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& mat) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, model_loc, mat);
}
//------------------------------------------------------------------------------
void surface_program::set_texture(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, texture_loc, unit);
}
//------------------------------------------------------------------------------
void surface_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
void surface_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "TexCoord");
}
//------------------------------------------------------------------------------
void surface_program::bind_occlusion_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Occlusion");
}
//------------------------------------------------------------------------------
// hair program
//------------------------------------------------------------------------------
void hair_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> prog;
    gl.object_label(prog, "hair program");

    const auto prog_src{
      embed(EAGINE_ID(HairProg), "furry_torus_hair.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "PrevModel") >> prev_model_loc;
    gl.get_uniform_location(prog, "CurrModel") >> curr_model_loc;
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "Tex") >> texture_loc;
}
//------------------------------------------------------------------------------
void hair_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void hair_program::use(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.use_program(prog);
}
//------------------------------------------------------------------------------
void hair_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, camera_loc, camera.matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
void hair_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& prev,
  const oglplus::trfmat<4>& curr) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, prev_model_loc, prev);
    gl.set_uniform(prog, curr_model_loc, curr);
}
//------------------------------------------------------------------------------
void hair_program::set_texture(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, texture_loc, unit);
}
//------------------------------------------------------------------------------
void hair_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
void hair_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Normal");
}
//------------------------------------------------------------------------------
void hair_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "TexCoord");
}
//------------------------------------------------------------------------------
void hair_program::bind_occlusion_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Occlusion");
}
//------------------------------------------------------------------------------
// surface geometry
//------------------------------------------------------------------------------
void shape_surface::init(
  execution_context& ec,
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen) {
    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(glapi, gen);
    geometry_and_bindings::init(glapi, shape, ec.buffer());
}
//------------------------------------------------------------------------------
// hair geometry
//------------------------------------------------------------------------------
void shape_hair::init(
  execution_context& ec,
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen) {

    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(
      glapi,
      shapes::surface_points(
        gen, 256 * 1024, shapes::vertex_attrib_kind::occlusion, ec.as_parent()));
    geometry_and_bindings::init(glapi, shape, ec.buffer());
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
void shape_textures::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // zebra texture
    const auto zebra_tex_src{embed(EAGINE_ID(ZebraTex), "zebra_fur")};

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
      oglplus::texture_image_block(zebra_tex_src.unpack(ec)));
    gl.generate_mipmap(GL.texture_2d);

    // monkey texture
    const auto monkey_tex_src{embed(EAGINE_ID(MonkeyTex), "monkey")};

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
      oglplus::texture_image_block(monkey_tex_src.unpack(ec)));
    gl.generate_mipmap(GL.texture_2d);
}
//------------------------------------------------------------------------------
void shape_textures::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_textures(std::move(monkey));
    gl.delete_textures(std::move(zebra));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
