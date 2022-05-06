/// @example app/016_torus/resources.cpp
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
#include <eagine/shapes/scaled_wrap_coords.hpp>
#include <eagine/shapes/torus.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void torus_program::init(execution_context& ec, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.create_program() >> prog;

    const auto prog_src{embed(EAGINE_ID(prog), "checker_torus.oglpprog")};
    gl.build_program(prog, prog_src.unpack(ec));
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Camera") >> camera_loc;
}
//------------------------------------------------------------------------------
void torus_program::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_program(std::move(prog));
}
//------------------------------------------------------------------------------
void torus_program::set_projection(video_context& vc, orbiting_camera& camera) {
    if(camera.has_changed()) {
        const auto& gl = vc.gl_api();
        gl.set_uniform(prog, camera_loc, camera.matrix(vc.surface_aspect()));
    }
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

    // geometry
    vertex_attrib_bindings::init(
      {(shapes::vertex_attrib_kind::position / 3),
       (shapes::vertex_attrib_kind::normal / 3),
       (shapes::vertex_attrib_kind::wrap_coord / 0)});
    oglplus::shape_generator shape(
      glapi,
      shapes::scale_wrap_coords(
        shapes::unit_torus(attrib_kinds()), 36.F, 12.F, 1.F));
    geometry::init(glapi, shape, *this, ec.buffer());
}
//------------------------------------------------------------------------------
void torus_geometry::draw(execution_context&, video_context& vc) {
    const auto& glapi = vc.gl_api();
    geometry::use(glapi);
    geometry::draw(glapi);
}
//------------------------------------------------------------------------------
void torus_geometry::clean_up(video_context& vc) {
    geometry::clean_up(vc.gl_api());
}
//------------------------------------------------------------------------------
} // namespace eagine::app
