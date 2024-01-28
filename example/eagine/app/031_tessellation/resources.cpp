/// @example app/031_tessellation/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
sphere_program::sphere_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&sphere_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void sphere_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("CameraMatrix") >> camera_matrix_loc;
    info.get_uniform_location("CameraPosition") >> camera_position_loc;
    info.get_uniform_location("ViewportDimensions") >> viewport_dim_loc;
}
//------------------------------------------------------------------------------
void sphere_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto [width, height] = vc.surface_size();
    set(vc, camera_position_loc, camera.position());
    set(vc, camera_matrix_loc, camera.matrix(vc));
    set(vc, viewport_dim_loc, oglplus::vec2(width, height));
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
      oglplus::vertex_attrib_location(0),
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
