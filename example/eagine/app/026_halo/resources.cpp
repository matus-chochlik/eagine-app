/// @example app/026_halo/resources.cpp
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
void surface_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> _prog;
    gl.object_label(_prog, "surface program");

    const auto prog_src{embed<"SurfProg">("halo_surface.oglpprog")};
    gl.build_program(_prog, prog_src.unpack(ec));
    gl.use_program(_prog);

    gl.get_uniform_location(_prog, "Model") >> _model_loc;
    gl.get_uniform_location(_prog, "View") >> _view_loc;
    gl.get_uniform_location(_prog, "Projection") >> _projection_loc;

    vc.clean_up_later(*this);
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

    const auto prog_src{embed<"HaloProg">("halo_halo.oglpprog")};
    gl.build_program(_prog, prog_src.unpack(ec));
    gl.use_program(_prog);

    gl.get_uniform_location(_prog, "Model") >> _model_loc;
    gl.get_uniform_location(_prog, "View") >> _view_loc;
    gl.get_uniform_location(_prog, "Projection") >> _projection_loc;
    gl.get_uniform_location(_prog, "CameraPos") >> _camera_pos_loc;

    vc.clean_up_later(*this);
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
    auto& ctx = ec.main_context();
    const auto& args = ctx.args();
    vertex_attrib_bindings::init(
      {shapes::vertex_attrib_kind::position,
       shapes::vertex_attrib_kind::normal});
    std::shared_ptr<shapes::generator> gen;

    if(args.find("--icosahedron")) {
        gen = shapes::unit_icosahedron(attrib_kinds());
    } else if(args.find("--twisted-torus")) {
        gen = shapes::unit_twisted_torus(attrib_kinds());
    } else if(args.find("--torus")) {
        gen = shapes::unit_torus(attrib_kinds());
    }

    if(!gen) {
        const auto json_src{embed<"SphereJson">("twisted_sphere.json")};
        gen = shapes::from_value_tree(
          valtree::from_json_text(as_chars(json_src.unpack(ctx)), ctx), ctx);
    }

    oglplus::shape_generator shape(
      glapi, shapes::add_triangle_adjacency(std::move(gen), ctx));
    geometry::init(glapi, shape, *this, ec.buffer());

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
void shape_geometry::clean_up(video_context& vc) {
    geometry::clean_up(vc.gl_api());
}
//------------------------------------------------------------------------------
void shape_geometry::draw(video_context& vc) {
    const auto& glapi = vc.gl_api();
    geometry::use(glapi);
    geometry::draw(glapi);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
