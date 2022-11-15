/// @example app/032_translucent_arrow/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
depth_program::depth_program(execution_context& ctx)
  : gl_program_resource{url{"json:///DepthProg"}, ctx} {
    loaded.connect(make_callable_ref<&depth_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void depth_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> camera_loc;
}
//------------------------------------------------------------------------------
void depth_program::set_camera(video_context& vc, orbiting_camera& camera) {
    use(vc).set(vc, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
draw_program::draw_program(execution_context& ctx)
  : gl_program_resource{url{"json:///DrawProg"}, ctx} {
    loaded.connect(make_callable_ref<&draw_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void draw_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> camera_loc;
    info.get_uniform_location("LightPosition") >> light_pos_loc;
    info.get_uniform_location("DepthTexture") >> depth_tex_loc;
}
//------------------------------------------------------------------------------
void draw_program::set_depth_texture(
  video_context& vc,
  oglplus::gl_types::int_type tex_unit) {
    use(vc).set(vc, depth_tex_loc, tex_unit);
}
//------------------------------------------------------------------------------
void draw_program::set_camera(video_context& vc, orbiting_camera& camera) {
    use(vc).set(vc, camera_loc, camera.matrix(vc));
}
//------------------------------------------------------------------------------
void draw_program::update(execution_context& ec, video_context& vc) {
    rad += radians_(0.5F * ec.state().frame_duration().value());

    use(vc).set(
      vc,
      light_pos_loc,
      oglplus::vec3(cos(rad) * 5, sin(rad) * 7, sin(rad * 0.618F) * 8));
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void shape_geometry::init(
  const std::shared_ptr<shapes::generator>& gen,
  video_context& vc) {
    const auto& glapi = vc.gl_api();

    oglplus::shape_generator shape(glapi, gen);
    bound_sphere = shape.bounding_sphere();

    gl_geometry_and_bindings::init({shape, vc});
}
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
void depth_texture::init(execution_context&, video_context& vc) {
    const auto& gl = vc.gl_api();

    gl.gen_textures() >> tex;
}
//------------------------------------------------------------------------------
void depth_texture::clean_up(video_context& vc) {
    vc.gl_api().clean_up(std::move(tex));
}
//------------------------------------------------------------------------------
void depth_texture::reshape(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    const auto [width, height] = vc.surface_size();

    gl.active_texture(GL.texture0 + tex_unit);
    gl.bind_texture(GL.texture_rectangle, tex);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_min_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_rectangle, GL.texture_mag_filter, GL.nearest);
    gl.tex_image2d(
      GL.texture_rectangle,
      0,
      GL.depth_component,
      width,
      height,
      0,
      GL.depth_component,
      GL.float_,
      memory::const_block());

    gl.viewport(width, height);
    gl.clear(GL.depth_buffer_bit);
}
//------------------------------------------------------------------------------
void depth_texture::copy_from_fb(video_context& vc) {
    const auto [width, height] = vc.surface_size();
    const auto& [gl, GL] = vc.gl_api();

    gl.copy_tex_image2d(
      GL.texture_rectangle, 0, GL.depth_component, 0, 0, width, height, 0);
}
//------------------------------------------------------------------------------

} // namespace eagine::app
