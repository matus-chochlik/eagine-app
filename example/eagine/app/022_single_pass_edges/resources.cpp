/// @example application/022_single_pass_edges/resources.cpp
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
#include <eagine/shapes/array.hpp>
#include <eagine/shapes/centered.hpp>
#include <eagine/shapes/icosahedron.hpp>
#include <eagine/shapes/scaled.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void edges_program::init(execution_context& ec, video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& gl = glapi;

    gl.create_program() >> prog;

    const auto prog_src{embed(EAGINE_ID(prog), "single_pass_edges.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Projection") >> camera_loc;
    gl.get_uniform_location(prog, "ViewportDimensions") >> vp_dim_loc;
}
//------------------------------------------------------------------------------
void edges_program::use(video_context& vc) {
    vc.gl_api().use_program(prog);
}
//------------------------------------------------------------------------------
void edges_program::clean_up(video_context& vc) {
    vc.gl_api().delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void edges_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto [width, height] = vc.surface_size();
    const auto& glapi = vc.gl_api();
    glapi.set_uniform(prog, camera_loc, camera.matrix(vc));
    glapi.set_uniform(prog, vp_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
void edges_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    vc.gl_api().bind_attrib_location(prog, loc, "Position");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(video_context& vc) {
    geometry_and_bindings::init(
      shapes::center(eagine::shapes::ortho_array_xyz(
        shapes::scale(
          shapes::unit_icosahedron(shapes::vertex_attrib_kind::position),
          {0.5F, 0.5F, 0.5F}),
        {1.F, 1.F, 1.F},
        {3, 3, 3})),
      vc);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
