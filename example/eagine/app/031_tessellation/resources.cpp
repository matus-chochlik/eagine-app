/// @example app/031_tessellation/resources.cpp
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
#include <eagine/shapes/to_patches.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
void sphere_program::init(video_context& vc) {
    create(vc)
      .label(vc, "sphere program")
      .build(vc, embed("TessProg", "sphere_tessellation.oglpprog"))
      .use(vc)
      .query(vc, "CameraMatrix", camera_matrix_loc)
      .query(vc, "CameraPosition", camera_position_loc)
      .query(vc, "ViewportDimensions", viewport_dim_loc)
      .query(vc, "OffsetBlock", offset_blk_idx)
      .clean_up_later(vc);
}
//------------------------------------------------------------------------------
void sphere_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto [width, height] = vc.surface_size();
    set(vc, camera_position_loc, camera.position());
    set(vc, camera_matrix_loc, camera.matrix(vc));
    set(vc, viewport_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
void sphere_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void sphere_program::bind_offsets_block(
  video_context& vc,
  oglplus::gl_types::uint_type binding) {
    bind(vc, offset_blk_idx, binding);
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(video_context& vc) {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    oglplus::shape_generator shape(
      glapi,
      shapes::to_patches(
        shapes::unit_icosahedron(shapes::vertex_attrib_kind::position)));

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
      "positions",
      vc.parent().buffer());

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, "indices", vc.parent().buffer());

    // offsets
    const float d = 4.2F;
    const float h = float(count - 1) * 0.5F;

    std::vector<oglplus::gl_types::float_type> offset_data;
    offset_data.resize(std_size(count * count * count * 4));
    vc.parent().random_normal(cover(offset_data));
    auto p = offset_data.begin();

    for(int k = 0; k != count; ++k) {
        const float z = (float(k) - h) * d;
        for(int j = 0; j != count; ++j) {
            const float y = (float(j) - h) * d;
            for(int i = 0; i != count; ++i) {
                const float x = (float(i) - h) * d;
                *p++ += x;
                *p++ += y;
                *p++ += z;
                *p++ = 0.F;
            }
        }
    }

    gl.gen_buffers() >> offsets;
    gl.bind_buffer(GL.uniform_buffer, offsets);
    gl.object_label(offsets, "offsets");
    gl.bind_buffer_base(GL.uniform_buffer, 0, offsets);
    gl.buffer_data(GL.uniform_buffer, view(offset_data), GL.static_draw);

    vc.clean_up_later(*this);
}
//------------------------------------------------------------------------------
void icosahedron_geometry::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_buffers(std::move(offsets));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));
}
//------------------------------------------------------------------------------
void icosahedron_geometry::draw(video_context& vc) {
    const auto& glapi = vc.gl_api();
    glapi.bind_vertex_array(vao);
    draw_instanced_using_instructions(glapi, view(ops), count * count * count);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
