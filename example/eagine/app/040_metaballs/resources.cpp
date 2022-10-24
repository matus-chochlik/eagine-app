/// @example app/040_metaballs/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"
#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// volume domain
//------------------------------------------------------------------------------
void volume_domain::init(example& e) {
    const auto& [gl, GL] = e.video().gl_api();

    // vao
    gl.delete_vertex_arrays.later_by(e, _tetrahedrons);
    gl.gen_vertex_arrays() >> _tetrahedrons;
    gl.bind_vertex_array(_tetrahedrons);
    gl.object_label(_tetrahedrons, "domain VAO");

    // corner positions
    // clang-format off
    const auto corner_data = GL.int_.array(
      0, 0, 0, // A
      1, 0, 0, // B
      0, 1, 0, // C
      1, 1, 0, // D
      0, 0, 1, // E
      1, 0, 1, // F
      0, 1, 1, // G
      1, 1, 1);// H
    // clang-format on

    gl.delete_buffers.later_by(e, _corners);
    gl.gen_buffers() >> _corners;
    gl.bind_buffer(GL.array_buffer, _corners);
    gl.object_label(_configs, "domain grid corners");
    gl.buffer_data(GL.array_buffer, view(corner_data), GL.static_draw);
    gl.vertex_attrib_ipointer(corner_loc(), 3, GL.int_);
    gl.enable_vertex_attrib_array(corner_loc());

    // indices
    const unsigned A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7,
                   X = 255;
    // clang-format off
    const auto index_data = GL.unsigned_int_.array(
		A, B, D, H,
		B, A, F, H,
		E, A, G, H,
		E, F, A, H,
		A, C, G, H,
		C, A, D, H
	);
    // clang-format on

    gl.delete_buffers.later_by(e, _indices);
    gl.gen_buffers() >> _indices;
    gl.bind_buffer(GL.element_array_buffer, _indices);
    gl.object_label(_indices, "domain grid indices");
    gl.buffer_data(GL.element_array_buffer, view(index_data), GL.static_draw);

    // field sample values
    gl.delete_buffers.later_by(e, _field);
    gl.gen_buffers() >> _field;
    gl.bind_buffer(GL.shader_storage_buffer, _field);
    gl.object_label(_field, "domain field values");
    gl.bind_buffer_base(GL.shader_storage_buffer, field_binding(), _field);
    gl.buffer_storage(GL.shader_storage_buffer, vertex_count() * 32);

    // metaball parameters
    gl.delete_buffers.later_by(e, _metaballs);
    gl.gen_buffers() >> _metaballs;
    gl.bind_buffer(GL.shader_storage_buffer, _metaballs);
    gl.object_label(_metaballs, "domain metaball parameters");
    gl.bind_buffer_base(
      GL.shader_storage_buffer, metaballs_binding(), _metaballs);
    gl.buffer_storage(GL.shader_storage_buffer, 4096);
    gl.clear_buffer_sub_data(
      GL.shader_storage_buffer, 0, 1, oglplus::gl_types::uint_type(0U));

    // tetrahedron cut configurations
    // clang-format off
    const auto config_data = GL.unsigned_int_.array(
		X, X, X, X,  X, X, X, X,
		A, C, B, X,  D, D, D, X,
		B, D, A, X,  C, C, C, X,
		B, B, A, A,  C, D, C, D,
		A, D, C, X,  B, B, B, X,
		A, A, C, C,  B, D, B, D,
		A, A, D, D,  C, B, C, B,
		A, A, A, X,  B, D, C, X,
		C, D, B, X,  A, A, A, X,
		C, C, B, B,  A, D, A, D,
		B, D, B, D,  C, C, A, A,
		B, B, B, X,  C, D, A, X,
		C, D, C, D,  A, A, B, B,
		C, C, C, X,  A, D, B, X,
		D, D, D, X,  A, B, C, X,
		X, X, X, X,  X, X, X, X);
    // clang-format on
    gl.delete_buffers.later_by(e, _configs);
    gl.gen_buffers() >> _configs;
    gl.bind_buffer(GL.shader_storage_buffer, _configs);
    gl.object_label(_configs, "polygonization configurations");
    gl.bind_buffer_base(GL.shader_storage_buffer, configs_binding(), _configs);
    gl.buffer_data(GL.shader_storage_buffer, view(config_data), GL.static_draw);
}
//------------------------------------------------------------------------------
void volume_domain::compute(example& e) {
    const auto& gl = e.video().gl_api();
    const auto groups = plane_count() / 4;
    gl.dispatch_compute(groups, groups, groups);
}
//------------------------------------------------------------------------------
void volume_domain::draw(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    gl.front_face(GL.ccw);
    gl.bind_vertex_array(_tetrahedrons);
    gl.draw_elements_instanced(
      GL.lines_adjacency, 4 * 6, GL.unsigned_int_, nullptr, cube_count());
}
//------------------------------------------------------------------------------
// metaball program
//------------------------------------------------------------------------------
metaball_program::metaball_program(example& e)
  : gl_program_resource{url{"json:///MBallProg"}, e.video(), e.loader()} {}
//------------------------------------------------------------------------------
// field program
//------------------------------------------------------------------------------
field_program::field_program(example& e)
  : gl_program_resource{url{"json:///FieldProg"}, e.video(), e.loader()} {}
//------------------------------------------------------------------------------
// surface program
//------------------------------------------------------------------------------
surface_program::surface_program(example& e)
  : gl_program_resource{url{"json:///SrfceProg"}, e.video(), e.loader()} {
    loaded.connect(make_callable_ref<&surface_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void surface_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("CameraMatrix") >> _camera_mat_loc;
    info.get_uniform_location("PerspectiveMatrix") >> _perspective_mat_loc;
}
//------------------------------------------------------------------------------
void surface_program::prepare_frame(example& e) {
    set(e.video(), _camera_mat_loc, e.camera().transform_matrix());
    set(
      e.video(),
      _perspective_mat_loc,
      e.camera().perspective_matrix(e.video().surface_aspect()));
}
//------------------------------------------------------------------------------
void surface_program::bind_corner_location(
  example& e,
  oglplus::vertex_attrib_location loc) {
    bind(e.video(), loc, "Corner");
}
//------------------------------------------------------------------------------
} // namespace eagine::app
