/// @example app/021_cel_shading/resources.cpp
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
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/icosahedron.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void cel_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> prog;

    const auto prog_src{embed(EAGINE_ID(prog), "cel_shading.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Projection") >> projection_loc;
    gl.get_uniform_location(prog, "Modelview") >> modelview_loc;
}
//------------------------------------------------------------------------------
void cel_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void cel_program::use(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.use_program(prog);
}
//------------------------------------------------------------------------------
void cel_program::set_projection(video_context& vc, orbiting_camera& camera) {
    if(camera.has_changed()) {
        vc.gl_api().set_uniform(prog, projection_loc, camera.matrix(vc));
    }
}
//------------------------------------------------------------------------------
void cel_program::set_modelview(execution_context& ec, video_context& vc) {
    shp_turns += 0.1F * ec.state().frame_duration().value();

    vc.gl_api().set_uniform(
      prog,
      modelview_loc,
      oglplus::matrix_rotation_x(turns_(shp_turns) / 1) *
        oglplus::matrix_rotation_y(turns_(shp_turns) / 2) *
        oglplus::matrix_rotation_z(turns_(shp_turns) / 3));
}
//------------------------------------------------------------------------------
void cel_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    vc.gl_api().bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(execution_context& ec, video_context& vc) {
    geometry_and_bindings::init(
      shapes::unit_icosahedron(shapes::vertex_attrib_kind::position), ec, vc);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
