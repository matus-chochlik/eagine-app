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

    gl.get_uniform_location(prog, "CameraPosition") >> camera_pos_loc;
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "Model") >> model_loc;
    gl.get_uniform_location(prog, "BrickMap") >> bricks_map_loc;
}
//------------------------------------------------------------------------------
void torus_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void torus_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto& gl = vc.gl_api();
    gl.set_uniform(
      prog, model_loc, oglplus::matrix_rotation_x(right_angles_(t))());
    gl.set_uniform(prog, camera_loc, camera.matrix(vc.surface_aspect()));
    gl.set_uniform(prog, camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
void torus_program::set_bricks_map(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    vc.gl_api().set_uniform(prog, bricks_map_loc, unit);
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
    const auto& gl = glapi;

    oglplus::shape_generator shape(
      glapi,
      shapes::unit_torus(
        shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal |
          shapes::vertex_attrib_kind::tangential |
          shapes::vertex_attrib_kind::wrap_coord,
        48,
        72,
        0.5F));

    auto draw_var = shape.draw_variant(0);
    ops.resize(std_size(shape.operation_count(draw_var)));
    shape.instructions(glapi, draw_var, cover(ops));

    // vao
    gl.gen_vertex_arrays() >> vao;
    gl.bind_vertex_array(vao);

    // positions
    gl.gen_buffers() >> positions;
    shape.attrib_setup(
      glapi,
      vao,
      positions,
      position_loc(),
      eagine::shapes::vertex_attrib_kind::position,
      ec.buffer());

    // normals
    gl.gen_buffers() >> normals;
    shape.attrib_setup(
      glapi,
      vao,
      normals,
      normal_loc(),
      eagine::shapes::vertex_attrib_kind::normal,
      ec.buffer());

    // tangents
    gl.gen_buffers() >> tangents;
    shape.attrib_setup(
      glapi,
      vao,
      tangents,
      tangent_loc(),
      eagine::shapes::vertex_attrib_kind::tangential,
      ec.buffer());

    // texcoords
    gl.gen_buffers() >> texcoords;
    shape.attrib_setup(
      glapi,
      vao,
      texcoords,
      texcoord_loc(),
      eagine::shapes::vertex_attrib_kind::wrap_coord,
      ec.buffer());

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, draw_var, ec.buffer());
}
//------------------------------------------------------------------------------
void torus_geometry::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(texcoords));
    gl.delete_buffers(std::move(tangents));
    gl.delete_buffers(std::move(normals));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));
}
//------------------------------------------------------------------------------
void torus_geometry::draw(execution_context&, video_context& ec) {
    draw_using_instructions(ec.gl_api(), view(ops));
}
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
void torus_textures::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // color texture
    const auto tex_src{embed(EAGINE_ID(BricksTex), "bricks")};

    gl.gen_textures() >> color_hmap_nmap;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d_array, color_hmap_nmap);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
    glapi.spec_tex_image3d(
      GL.texture_2d_array,
      0,
      0,
      oglplus::texture_image_block(tex_src.unpack(ec)));
    gl.generate_mipmap(GL.texture_2d_array);
}
//------------------------------------------------------------------------------
void torus_textures::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_textures(std::move(color_hmap_nmap));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
