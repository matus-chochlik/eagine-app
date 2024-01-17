/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app.tiling_viewer;

import eagine.core;
import eagine.msgbus;
import eagine.guiplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
//  Application
//------------------------------------------------------------------------------
auto tiling_viewer::_show_settings_handler() noexcept {
    return make_callable_ref<&tiling_viewer::_show_settings>(this);
}
//------------------------------------------------------------------------------
void tiling_viewer::_init_inputs() {
    _camera.connect_inputs(context()).basic_input_mapping(context());
    context()
      .connect_inputs()
      .add_ui_button({"Button", "ShowSeting"}, "Settings")
      .connect_input({"Viewer", "ShowSeting"}, _show_settings_handler())
      .map_key({"Viewer", "ShowSeting"}, {"Q"})
      .map_input(
        {"Viewer", "ShowSeting"},
        {"AppGUI"},
        {"Button", "ShowSeting"},
        input_setup().any_value_kind())
      .map_inputs();
    context().setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void tiling_viewer::_init_camera(const oglplus::sphere bs) {
    const auto sr{std::max(bs.radius(), 0.1F)};
    _camera.set_fov(degrees_(_fov))
      .set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 100.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 25.0F);
}
//------------------------------------------------------------------------------
tiling_viewer::tiling_viewer(execution_context& ctx, video_context& video)
  : common_application{ctx}
  , _video{video}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.0F}, {0.4F, 0.4F, 0.4F, 0.0F}, 1.F} {
    _init_camera({{0.F, 0.F, 0.F}, 1.F});
    _init_inputs();
}
//------------------------------------------------------------------------------
void tiling_viewer::_show_settings(const input& i) noexcept {
    if(not i) {
        _show_setting_window = true;
    }
}
//------------------------------------------------------------------------------
auto tiling_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void tiling_viewer::_setting_window(const guiplus::imgui_api& gui) noexcept {
    const auto height{85.F};
    gui.set_next_window_size({350, height});
    if(gui.slider_float("FOV", _fov, 20.F, 120.F)) {
        _camera.set_fov(degrees_(_fov));
    }
    gui.same_line();
    gui.help_marker("changes the field of view of the camera");

    gui.separator();
    if(gui.button("Close").or_true()) {
        _show_setting_window = false;
    }
    gui.same_line();
    gui.help_marker("closes this settings window");
    gui.end();
}
//------------------------------------------------------------------------------
void tiling_viewer::update_overlays(guiplus::gui_utils& utils) noexcept {
    if(_show_setting_window) {
        _setting_window(utils.imgui);
    }
}
//------------------------------------------------------------------------------
void tiling_viewer::update() noexcept {
    _bg.clear(_video, _camera);
    _video.commit();
}
//------------------------------------------------------------------------------
void tiling_viewer::clean_up() noexcept {
    _bg.clean_up(_video);
    _video.end();
}
//------------------------------------------------------------------------------
} // namespace eagine::app

