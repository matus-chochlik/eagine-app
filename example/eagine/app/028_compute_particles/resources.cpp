/// @example app/028_compute_particles/resources.cpp
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
// particles
//------------------------------------------------------------------------------
void particles::init(example& e) {
    const auto& [gl, GL] = e.video().gl_api();

    // vao
    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    // origin
    const auto origin_data = GL.float_.array(0.F, 0.F, 0.F);

    gl.gen_buffers() >> _origin;
    gl.bind_buffer(GL.array_buffer, _origin);
    gl.object_label(_origin, "origin");
    gl.buffer_data(GL.array_buffer, view(origin_data), GL.static_draw);
    gl.vertex_attrib_pointer(origin_loc(), 3, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(origin_loc());

    // cursors
    const auto cursor_values = GL.unsigned_int_.array(0U, 0U);

    gl.gen_buffers() >> _cursors;
    gl.bind_buffer(GL.atomic_counter_buffer, _cursors);
    gl.object_label(_cursors, "cursors");
    gl.bind_buffer_base(GL.atomic_counter_buffer, cursors_binding(), _cursors);
    gl.buffer_data(
      GL.atomic_counter_buffer, view(cursor_values), GL.dynamic_draw);

    //
    std::vector<oglplus::gl_types::float_type> init_data;
    // random
    init_data.resize(std_size(_count * 16));
    e.context().random_uniform_01(cover(init_data));

    gl.gen_buffers() >> _random;
    gl.bind_buffer(GL.shader_storage_buffer, _random);
    gl.object_label(_random, "random");
    gl.bind_buffer_base(GL.shader_storage_buffer, random_binding(), _random);
    gl.buffer_data(GL.shader_storage_buffer, view(init_data), GL.static_draw);

    // offsets
    init_data.resize(std_size(_count * 4));
    fill(cover(init_data), 0.F);
    gl.gen_buffers() >> _offsets;
    gl.bind_buffer(GL.shader_storage_buffer, _offsets);
    gl.object_label(_offsets, "offsets");
    gl.bind_buffer_base(GL.shader_storage_buffer, offsets_binding(), _offsets);
    gl.buffer_data(GL.shader_storage_buffer, view(init_data), GL.dynamic_draw);

    // velocities
    init_data.resize(std_size(_count * 4));
    fill(cover(init_data), 0.F);
    gl.gen_buffers() >> _velocities;
    gl.bind_buffer(GL.shader_storage_buffer, _velocities);
    gl.object_label(_velocities, "velocities");
    gl.bind_buffer_base(
      GL.shader_storage_buffer, velocities_binding(), _velocities);
    gl.buffer_data(GL.shader_storage_buffer, view(init_data), GL.dynamic_draw);

    // ages
    init_data.resize(std_size(_count));
    fill(cover(init_data), 1.F);
    gl.gen_buffers() >> _ages;
    gl.bind_buffer(GL.shader_storage_buffer, _ages);
    gl.object_label(_ages, "ages");
    gl.bind_buffer_base(GL.shader_storage_buffer, ages_binding(), _ages);
    gl.buffer_data(GL.shader_storage_buffer, view(init_data), GL.dynamic_draw);
}
//------------------------------------------------------------------------------
void particles::emit(example& e) {
    const auto& [gl, GL] = e.video().gl_api();

    const auto cursor_values = GL.unsigned_int_.array(0U);
    gl.buffer_sub_data(GL.atomic_counter_buffer, 0, view(cursor_values));
    gl.dispatch_compute(_count, 1, 1);
}
//------------------------------------------------------------------------------
void particles::draw(example& e) {
    const auto& [gl, GL] = e.video().gl_api();
    gl.draw_arrays_instanced(GL.points, 0, 1, signedness_cast(_count));
}
//------------------------------------------------------------------------------
void particles::clean_up(example& e) noexcept {
    const auto& gl = e.video().gl_api().operations();
    gl.delete_buffers(std::move(_ages));
    gl.delete_buffers(std::move(_velocities));
    gl.delete_buffers(std::move(_offsets));
    gl.delete_buffers(std::move(_random));
    gl.delete_buffers(std::move(_origin));
    gl.delete_vertex_arrays(std::move(_vao));
}
//------------------------------------------------------------------------------
// emit program
//------------------------------------------------------------------------------
emit_program::emit_program(example& e)
  : gl_program_resource{url{"json:///EmitProg"}, e.video(), e.loader()} {
    loaded.connect(make_callable_ref<&emit_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void emit_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("emitPosition") >> _emit_position_loc;
    info.get_uniform_location("deltaTime") >> _delta_time_loc;
}
//------------------------------------------------------------------------------
void emit_program::prepare_frame(example& e) {
    use(e.context());
    const auto& gl = e.video().gl_api();
    gl.set_uniform(*this, _emit_position_loc, e.emit_position());
    gl.set_uniform(*this, _delta_time_loc, e.frame_duration());
}
//------------------------------------------------------------------------------
void emit_program::bind_random(
  example& e,
  oglplus::gl_types::uint_type binding) {
    const auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "RandomBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
void emit_program::bind_offsets(
  example& e,
  oglplus::gl_types::uint_type binding) {
    const auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "OffsetBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
void emit_program::bind_velocities(
  example& e,
  oglplus::gl_types::uint_type binding) {
    const auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "VelocityBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
void emit_program::bind_ages(example& e, oglplus::gl_types::uint_type binding) {
    const auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "AgeBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
// draw program
//------------------------------------------------------------------------------
draw_program::draw_program(example& e)
  : gl_program_resource{url{"json:///DrawProg"}, e.video(), e.loader()} {
    loaded.connect(make_callable_ref<&draw_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void draw_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("CameraMatrix") >> _camera_mat_loc;
    info.get_uniform_location("PerspectiveMatrix") >> _perspective_mat_loc;
}
//------------------------------------------------------------------------------
void draw_program::prepare_frame(example& e) {
    use(e.context());
    e.video().gl_api().set_uniform(
      *this, _camera_mat_loc, e.camera().transform_matrix());
    e.video().gl_api().set_uniform(
      *this,
      _perspective_mat_loc,
      e.camera().perspective_matrix(e.video().surface_aspect()));
}
//------------------------------------------------------------------------------
void draw_program::bind_origin_location(
  example& e,
  oglplus::vertex_attrib_location loc) {
    e.video().gl_api().bind_attrib_location(*this, loc, "Origin");
}
//------------------------------------------------------------------------------
void draw_program::bind_offsets(
  example& e,
  oglplus::gl_types::uint_type binding) {
    auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "OffsetBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
void draw_program::bind_ages(example& e, oglplus::gl_types::uint_type binding) {
    const auto& gl = e.video().gl_api();
    oglplus::shader_storage_block_index blk_idx;
    gl.get_shader_storage_block_index(*this, "AgeBlock") >> blk_idx;
    gl.shader_storage_block_binding(*this, blk_idx, binding);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
