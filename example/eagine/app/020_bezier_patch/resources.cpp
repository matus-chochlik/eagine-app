/// @example app/020_bezier_patch/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
patch_program::patch_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&patch_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void patch_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("CameraMatrix") >> camera_matrix_loc;
    info.get_uniform_location("PerspectiveMatrix") >> perspective_matrix_loc;
    info.get_uniform_location("Color") >> color_loc;
}
//------------------------------------------------------------------------------
void patch_program::set_projection(
  video_context& video,
  orbiting_camera& camera) {
    set(video, camera_matrix_loc, camera.transform_matrix());
    set(
      video,
      perspective_matrix_loc,
      camera.perspective_matrix(video.surface_aspect()));
}
//------------------------------------------------------------------------------
void patch_program::set_wireframe_color(video_context& video) {
    set(video, color_loc, oglplus::vec4(0.1F, 0.1F, 0.1F, 1.0F));
}
//------------------------------------------------------------------------------
void patch_program::set_surface_color(video_context& video) {
    set(video, color_loc, oglplus::vec4(1.0F, 1.0F, 1.0F, 0.4F));
}
//------------------------------------------------------------------------------
void patch_program::bind_position_location(
  video_context& video,
  oglplus::vertex_attrib_location loc) {
    bind(video, loc, "Position");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void patch_geometry::init(execution_context& ec) {
    const auto& [gl, GL] = ec.main_video().gl_api();

    // vao
    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    // positions
    // clang-format off
    const auto position_data = GL.float_.array(
		-2.F,  0.F, -2.F,
		-1.F,  0.F, -3.F,
		 1.F,  0.F, -5.F,
		 2.F,  0.F, -2.F,
		-1.F,  0.F, -1.F,
		 0.F,  4.F, -1.F,
		 1.F,  4.F, -1.F,
		 3.F,  0.F, -1.F,
		-1.F,  0.F,  1.F,
		-1.F,  4.F,  1.F,
		 0.F,  4.F,  1.F,
		 1.F,  0.F,  1.F,
		-2.F,  0.F,  2.F,
		-1.F,  0.F,  5.F,
		 1.F,  0.F,  3.F,
		 2.F,  0.F,  2.F);
    // clang-format on

    gl.gen_buffers() >> _positions;
    gl.bind_buffer(GL.array_buffer, _positions);
    gl.buffer_data(GL.array_buffer, view(position_data), GL.static_draw);
    gl.vertex_attrib_pointer(position_loc(), 3, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(position_loc());
}
//------------------------------------------------------------------------------
void patch_geometry::clean_up(execution_context& ec) {
    const auto& gl = ec.main_video().gl_api();
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
}
//------------------------------------------------------------------------------
void patch_geometry::draw(execution_context& ec) {
    const auto& [gl, GL] = ec.main_video().gl_api();
    gl.draw_arrays(GL.patches, 0, 16);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
