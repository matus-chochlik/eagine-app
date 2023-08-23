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
auto model_viewer::_initial_geometry() -> model_viewer_geometry_holder {
    return make_default_geometry(context(), _video);
}
//------------------------------------------------------------------------------
auto model_viewer::_initial_program() -> model_viewer_program_holder {
    return make_default_program(context(), _video);
}
//------------------------------------------------------------------------------
model_viewer::model_viewer(execution_context& ec, video_context& vc)
  : common_application{ec}
  , _video{vc}
  , _geometry{_initial_geometry()}
  , _program{_initial_program()} {
    _geometry.loaded.connect(make_callable_ref<&model_viewer::on_loaded>(this));
    _program.loaded.connect(make_callable_ref<&model_viewer::on_loaded>(this));

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // camera
    // TODO
    const auto sr = 10.F;
    _camera.set_fov(right_angle_())
      .set_near(sr * 0.1F)
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 2.0F)
      .set_orbit_max(sr * 4.0F);

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void model_viewer::on_loaded(model_viewer_resource_intf&) noexcept {
    // TODO
    std::cout << "LOADED" << std::endl;
}
//------------------------------------------------------------------------------
auto model_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void model_viewer::view_model() noexcept {
    auto& state = context().state();
    _camera.idle_update(state);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
}
//------------------------------------------------------------------------------
void model_viewer::update() noexcept {
    if(_geometry and _program) {
        view_model();
    } else {
        _geometry.load_if_needed(context());
        _program.load_if_needed(context());
    }
    _video.commit();
}
//------------------------------------------------------------------------------
void model_viewer::clean_up() noexcept {
    _program.clean_up(context(), _video);
    _geometry.clean_up(context(), _video);
    _video.end();
}
//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad>;
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
