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
auto model_viewer::_initial_background() -> model_viewer_background_holder {
    return make_default_background(context(), _video);
}
//------------------------------------------------------------------------------
auto model_viewer::_initial_geometry() -> model_viewer_geometry_holder {
    return make_default_geometry(context(), _video);
}
//------------------------------------------------------------------------------
auto model_viewer::_initial_program() -> model_viewer_program_holder {
    return make_default_program(context(), _video);
}
//------------------------------------------------------------------------------
void model_viewer::_init_camera(const oglplus::sphere bs) {
    const auto sr{bs.radius()};
    _camera.set_fov(right_angle_())
      .set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 4.0F);
}
//------------------------------------------------------------------------------
model_viewer::model_viewer(execution_context& ec, video_context& vc)
  : common_application{ec}
  , _video{vc}
  , _background{_initial_background()}
  , _geometry{_initial_geometry()}
  , _program{_initial_program()} {
    _background.loaded.connect(_load_handler());
    _geometry.loaded.connect(_load_handler());
    _program.loaded.connect(_load_handler());

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void model_viewer::_on_loaded(model_viewer_resource_intf&) noexcept {
    if(_background and _geometry and _program) {
        _init_camera(_geometry.bounding_sphere());
        _program.use(_video);
        _program.apply_bindings(_video, _geometry.attrib_bindings());
    }
}
//------------------------------------------------------------------------------
auto model_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void model_viewer::clear_background() noexcept {
    _background.use(_video);
    _background.clear(_video, _camera);
}
//------------------------------------------------------------------------------
void model_viewer::view_model() noexcept {
    _program.use(_video);
    _program.set_camera(_video, _camera);

    _geometry.use(_video);
    _geometry.draw(_video);
}
//------------------------------------------------------------------------------
void model_viewer::update() noexcept {
    auto& state = context().state();
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    if(_background) {
        clear_background();
    } else {
        _background.load_if_needed(context());
    }

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
