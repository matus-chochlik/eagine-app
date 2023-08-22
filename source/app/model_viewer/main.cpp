/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
//  Application
//------------------------------------------------------------------------------
auto model_viewer::_initial_program(execution_context& ctx, video_context& video)
  -> unique_holder<model_viewer_program_intf> {
    return make_default_program(ctx, video);
}
//------------------------------------------------------------------------------
model_viewer::model_viewer(execution_context& ec, video_context& vc)
  : common_application{ec}
  , _video{vc}
  , _prog{_initial_program(ec, vc)} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // camera
    // TODO
    const auto sr = 10.F;
    camera.set_fov(right_angle_())
      .set_near(sr * 0.1F)
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 2.0F)
      .set_orbit_max(sr * 4.0F);

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
auto model_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void model_viewer::update() noexcept {
    auto& state = context().state();
    camera.idle_update(state);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);

    _video.commit();
}
//------------------------------------------------------------------------------
void model_viewer::clean_up() noexcept {
    _prog.clean_up(context(), _video);

    _video.end();
}
//------------------------------------------------------------------------------
//  Launchpad
//------------------------------------------------------------------------------
class model_viewer_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final;
    auto check_requirements(video_context& vc) -> bool final;

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final;
};
//------------------------------------------------------------------------------
auto model_viewer_launchpad::setup(main_ctx&, launch_options& opts) -> bool {
    opts.no_audio().require_input().require_video();
    return true;
}
//------------------------------------------------------------------------------
auto model_viewer_launchpad::launch(execution_context& ec, const launch_options&)
  -> unique_holder<application> {
    return launch_with_video<model_viewer>(ec);
}
//------------------------------------------------------------------------------
auto model_viewer_launchpad::check_requirements(video_context& vc) -> bool {
    const auto& [gl, GL] = vc.gl_api();

    return gl.disable and gl.clear_color and gl.create_shader and
           gl.shader_source and gl.compile_shader and gl.create_program and
           gl.attach_shader and gl.link_program and gl.use_program and
           gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
           gl.gen_vertex_arrays and gl.bind_vertex_array and
           gl.get_attrib_location and gl.vertex_attrib_pointer and
           gl.enable_vertex_attrib_array and gl.draw_arrays and
           GL.vertex_shader and GL.geometry_shader and GL.fragment_shader;
}
//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad> {
    return {hold<model_viewer_launchpad>};
}
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
