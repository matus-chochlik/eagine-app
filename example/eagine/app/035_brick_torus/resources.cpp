/// @example app/035_brick_torus/resources.cpp
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
#include <eagine/shapes/torus.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void torus_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> prog;

    const auto prog_src{embed(EAGINE_ID(prog), "brick_torus.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "LightPosition") >> light_pos_loc;
    gl.get_uniform_location(prog, "CameraPosition") >> camera_pos_loc;
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "Model") >> model_loc;
    gl.get_uniform_location(prog, "TextureMap") >> texture_map_loc;
}
//------------------------------------------------------------------------------
void torus_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void torus_program::set_camera(video_context& vc, orbiting_camera& camera) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, camera_loc, camera.matrix(vc.surface_aspect()));
    gl.set_uniform(prog, camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
void torus_program::set_model(
  video_context& vc,
  const oglplus::trfmat<4>& model) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, model_loc, model);
}
//------------------------------------------------------------------------------
void torus_program::set_light(video_context& vc, const oglplus::vec3& light) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(prog, light_pos_loc, light);
}
//------------------------------------------------------------------------------
void torus_program::set_texture_map(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    vc.gl_api().set_uniform(prog, texture_map_loc, unit);
}
//------------------------------------------------------------------------------
void torus_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
void torus_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Normal");
}
//------------------------------------------------------------------------------
void torus_program::bind_tangent_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "Tangent");
}
//------------------------------------------------------------------------------
void torus_program::bind_texcoord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(prog, loc, "TexCoord");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void torus_geometry::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(
      glapi,
      shapes::unit_torus(
        shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal |
          shapes::vertex_attrib_kind::tangent |
          shapes::vertex_attrib_kind::wrap_coord,
        48,
        72,
        0.5F));
    geometry_and_bindings::init(glapi, shape, ec.buffer());
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
void torus_textures::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // bricks texture
    const auto brick_tex_src{embed(EAGINE_ID(BricksTex), "bricks")};

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
      oglplus::texture_image_block(brick_tex_src.unpack(ec)));
    gl.generate_mipmap(GL.texture_2d_array);

    // stones texture
    const auto stones_tex_src{embed(EAGINE_ID(StonesTex), "stones")};

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
      oglplus::texture_image_block(stones_tex_src.unpack(ec)));
    gl.generate_mipmap(GL.texture_2d_array);
}
//------------------------------------------------------------------------------
void torus_textures::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_textures(std::move(stones));
    gl.delete_textures(std::move(bricks));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
