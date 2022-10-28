/// @example application/019_scaled_cubes/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

import <cmath>;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
cubes_program::cubes_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx.loader(), ctx.main_video()} {
    loaded.connect(make_callable_ref<&cubes_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void cubes_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> camera_loc;
    info.get_uniform_location("Center") >> center_loc;
    info.get_uniform_location("Time") >> time_loc;
    info.get_uniform_location("Edges") >> edges_loc;
}
//------------------------------------------------------------------------------
void cubes_program::set_projection(
  video_context& video,
  orbiting_camera& camera) {
    if(camera.has_changed()) {
        set(video, camera_loc, camera.matrix(video.surface_aspect()));
    }
}
//------------------------------------------------------------------------------
void cubes_program::update(execution_context& ec, video_context& video) {
    const auto t = ec.state().frame_time().value();
    set(
      video,
      center_loc,
      oglplus::to_cartesian(oglplus::unit_spherical_coordinate(
        turns_(t / 3.F),
        oglplus::smooth_lerp(
          right_angles_(1.F), right_angles_(-1.F), t / 5.F))) *
        oglplus::smooth_lerp(0.F, 10.F, t / 7.F));
    set(video, time_loc, t);
}
//------------------------------------------------------------------------------
void cubes_program::drawing_surface(video_context& video) {
    set(video, edges_loc, 0.F);
}
//------------------------------------------------------------------------------
void cubes_program::drawing_edges(video_context& video) {
    set(video, edges_loc, 1.F);
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void cubes_geometry::init(execution_context& ec, video_context& video) {
    const auto& glapi = video.gl_api();
    const auto& gl = glapi;

    oglplus::shape_generator shape(
      glapi,
      shapes::rebox(shapes::center(eagine::shapes::ortho_array_xyz(
        shapes::unit_cube(
          shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::pivot),
        {1.F, 1.F, 1.F},
        {10, 10, 10}))));

    std::array<shapes::drawing_variant, 2> vars{
      shape.draw_variant(0), shape.draw_variant(1)};

    ops.resize(std_size(shape.operation_count(view(vars))));
    shape.instructions(glapi, view(vars), cover(subs), cover(ops));

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

    // pivots
    gl.gen_buffers() >> pivots;
    shape.attrib_setup(
      glapi,
      vao,
      pivots,
      pivot_loc(),
      eagine::shapes::vertex_attrib_kind::pivot,
      ec.buffer());

    // coords
    gl.gen_buffers() >> coords;
    shape.attrib_setup(
      glapi,
      vao,
      coords,
      coord_loc(),
      eagine::shapes::vertex_attrib_kind::box_coord,
      ec.buffer());

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, view(vars), ec.buffer());
}
//------------------------------------------------------------------------------
void cubes_geometry::clean_up(execution_context& ec) {
    const auto& gl = ec.main_video().gl_api();
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(coords));
    gl.delete_buffers(std::move(pivots));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));
}
//------------------------------------------------------------------------------
void cubes_geometry::draw_surface(video_context& vc) {
    draw_using_instructions(vc.gl_api(), view(ops), subs[0]);
}
//------------------------------------------------------------------------------
void cubes_geometry::draw_edges(video_context& vc) {
    draw_using_instructions(vc.gl_api(), view(ops), subs[1]);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
