/// @example app/026_halo/resources.cpp
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
#include <eagine/oglplus/math/matrix_ctrs.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/adjacency.hpp>
#include <eagine/shapes/icosahedron.hpp>
#include <eagine/shapes/torus.hpp>
#include <eagine/shapes/twisted_torus.hpp>
#include <eagine/shapes/value_tree.hpp>
#include <eagine/value_tree/json.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// surface program
//------------------------------------------------------------------------------
void surface_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> _prog;
    gl.object_label(_prog, "surface program");

    const auto prog_src{embed(EAGINE_ID(SurfProg), "halo_surface.oglpprog")};
    gl.build_program(_prog, prog_src.unpack(ec));
    gl.use_program(_prog);

    gl.get_uniform_location(_prog, "Model") >> _model_loc;
    gl.get_uniform_location(_prog, "View") >> _view_loc;
    gl.get_uniform_location(_prog, "Projection") >> _projection_loc;
}
//------------------------------------------------------------------------------
void surface_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(_prog));
}
//------------------------------------------------------------------------------
void surface_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto& gl = vc.gl_api();
    gl.use_program(_prog);
    gl.set_uniform(
      _prog, _model_loc, oglplus::matrix_rotation_x(right_angles_(t))());
    gl.set_uniform(_prog, _view_loc, camera.transform_matrix());
    gl.set_uniform(
      _prog, _projection_loc, camera.perspective_matrix(vc.surface_aspect()));
}
//------------------------------------------------------------------------------
void surface_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(_prog, loc, "Position");
}
//------------------------------------------------------------------------------
void surface_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(_prog, loc, "Normal");
}
//------------------------------------------------------------------------------
// halo program
//------------------------------------------------------------------------------
void halo_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> _prog;
    gl.object_label(_prog, "halo program");

    const auto prog_src{embed(EAGINE_ID(HaloProg), "halo_halo.oglpprog")};
    gl.build_program(_prog, prog_src.unpack(ec));
    gl.use_program(_prog);

    gl.get_uniform_location(_prog, "Model") >> _model_loc;
    gl.get_uniform_location(_prog, "View") >> _view_loc;
    gl.get_uniform_location(_prog, "Projection") >> _projection_loc;
    gl.get_uniform_location(_prog, "CameraPos") >> _camera_pos_loc;
}
//------------------------------------------------------------------------------
void halo_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(_prog));
}
//------------------------------------------------------------------------------
void halo_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto& gl = vc.gl_api();
    gl.use_program(_prog);

    gl.set_uniform(
      _prog, _model_loc, oglplus::matrix_rotation_x(right_angles_(t))());
    gl.set_uniform(_prog, _view_loc, camera.transform_matrix());
    gl.set_uniform(
      _prog, _projection_loc, camera.perspective_matrix(vc.surface_aspect()));
    gl.set_uniform(_prog, _camera_pos_loc, camera.position());
}
//------------------------------------------------------------------------------
void halo_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(_prog, loc, "Position");
}
//------------------------------------------------------------------------------
void halo_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    const auto& gl = vc.gl_api();
    gl.bind_attrib_location(_prog, loc, "Normal");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void shape_geometry::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& gl = glapi;

    auto& ctx = ec.main_context();
    const auto& args = ctx.args();
    std::shared_ptr<shapes::generator> gen;

    if(args.find("--icosahedron")) {
        gen = shapes::unit_icosahedron(
          shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal);
    } else if(args.find("--twisted-torus")) {
        gen = shapes::unit_twisted_torus(
          shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal);
    } else if(args.find("--torus")) {
        gen = shapes::unit_torus(
          shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal);
    }

    if(!gen) {
        const auto json_src{
          embed(EAGINE_ID(SphereJson), "twisted_sphere.json")};
        gen = shapes::from_value_tree(
          valtree::from_json_text(as_chars(json_src.unpack(ctx)), ctx), ctx);
    }

    oglplus::shape_generator shape(
      glapi, shapes::add_triangle_adjacency(std::move(gen), ctx));

    auto draw_var = shape.draw_variant(0);
    _ops.resize(std_size(shape.operation_count(draw_var)));
    shape.instructions(glapi, draw_var, cover(_ops));

    // vao
    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    // positions
    gl.gen_buffers() >> _positions;
    shape.attrib_setup(
      glapi,
      _vao,
      _positions,
      position_loc(),
      eagine::shapes::vertex_attrib_kind::position,
      "positions",
      ec.buffer());

    // normals
    gl.gen_buffers() >> _normals;
    shape.attrib_setup(
      glapi,
      _vao,
      _normals,
      normal_loc(),
      eagine::shapes::vertex_attrib_kind::normal,
      "normals",
      ec.buffer());

    // indices
    gl.gen_buffers() >> _indices;
    shape.index_setup(glapi, _indices, draw_var, "indices", ec.buffer());
}
//------------------------------------------------------------------------------
void shape_geometry::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_normals));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
}
//------------------------------------------------------------------------------
void shape_geometry::draw(execution_context&, video_context& ec) {
    draw_using_instructions(ec.gl_api(), view(_ops));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
