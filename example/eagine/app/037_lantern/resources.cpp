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
#include <eagine/shapes/value_tree.hpp>
#include <eagine/value_tree/json.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// draw program
//------------------------------------------------------------------------------
void draw_program::init(video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    // vertex shader
    auto vs_source = embed(EAGINE_ID(VertShader), "vertex.glsl");
    oglplus::owned_shader_name vs;
    const auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.create_shader(GL.vertex_shader) >> vs;
    gl.shader_source(
      vs, oglplus::glsl_string_ref(vs_source.unpack(vc.parent())));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed(EAGINE_ID(FragShader), "fragment.glsl");
    oglplus::owned_shader_name fs;
    const auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.create_shader(GL.fragment_shader) >> fs;
    gl.shader_source(
      fs, oglplus::glsl_string_ref(fs_source.unpack(vc.parent())));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "LightPower") >> light_power_loc;
    gl.get_uniform_location(prog, "Tex") >> tex_loc;
}
//------------------------------------------------------------------------------
void draw_program::set_camera(video_context& vc, const orbiting_camera& camera) {
    vc.gl_api().set_uniform(prog, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void draw_program::set_light_power(
  video_context& vc,
  oglplus::gl_types::float_type value) {
    vc.gl_api().set_uniform(prog, light_power_loc, value);
}
//------------------------------------------------------------------------------
void draw_program::set_texture_unit(
  video_context& vc,
  oglplus::gl_types::int_type unit) {
    vc.gl_api().set_uniform(prog, tex_loc, unit);
}
//------------------------------------------------------------------------------
void draw_program::clean_up(video_context& vc) {
    vc.gl_api().delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void draw_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location position_loc) {
    vc.gl_api().bind_attrib_location(prog, position_loc, "Position");
}
//------------------------------------------------------------------------------
void draw_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location normal_loc) {
    vc.gl_api().bind_attrib_location(prog, normal_loc, "Normal");
}
//------------------------------------------------------------------------------
void draw_program::bind_wrap_coord_location(
  video_context& vc,
  oglplus::vertex_attrib_location coord_loc) {
    vc.gl_api().bind_attrib_location(prog, coord_loc, "WrapCoord");
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
    bound_sphere = shape.bounding_sphere();

    ops.resize(std_size(shape.operation_count()));
    shape.instructions(glapi, cover(ops));

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
      vc.parent().buffer());

    // normals
    gl.gen_buffers() >> normals;
    shape.attrib_setup(
      glapi,
      vao,
      normals,
      normal_loc(),
      eagine::shapes::vertex_attrib_kind::normal,
      vc.parent().buffer());

    // wrap_coords
    gl.gen_buffers() >> wrap_coords;
    shape.attrib_setup(
      glapi,
      vao,
      wrap_coords,
      wrap_coord_loc(),
      eagine::shapes::vertex_attrib_kind::wrap_coord,
      vc.parent().buffer());

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, vc.parent().buffer());

    // textures
    const auto tex_src{embed(EAGINE_ID(PumpkinTex), "pumpkin")};

    gl.gen_textures() >> tex;
    gl.active_texture(GL.texture0 + tex_unit());
    gl.bind_texture(GL.texture_2d_array, tex);
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
}
//------------------------------------------------------------------------------
void pumpkin_geometry::use(video_context& vc) {
    vc.gl_api().bind_vertex_array(vao);
}
//------------------------------------------------------------------------------
void pumpkin_geometry::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.delete_textures(std::move(tex));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(wrap_coords));
    gl.delete_buffers(std::move(normals));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));
}
//------------------------------------------------------------------------------
void pumpkin_geometry::draw(video_context& vc) {
    oglplus::draw_using_instructions(vc.gl_api(), view(ops));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
