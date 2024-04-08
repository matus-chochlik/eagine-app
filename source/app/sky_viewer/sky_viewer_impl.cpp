/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app.sky_viewer;

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
auto sky_viewer::_load_handler() noexcept {
    return make_callable_ref<&sky_viewer::_on_loaded>(this);
}
//------------------------------------------------------------------------------
auto sky_viewer::_select_handler() noexcept {
    return make_callable_ref<&sky_viewer::_on_selected>(this);
}
//------------------------------------------------------------------------------
auto sky_viewer::_show_settings_handler() noexcept {
    return make_callable_ref<&sky_viewer::_show_settings>(this);
}
//------------------------------------------------------------------------------
void sky_viewer::_init_inputs() {
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
void sky_viewer::_init_camera() {
    const auto sr{1.0};
    _camera.set_fov(degrees_(_fov))
      .set_orbit_min(sr * 1.414F)
      .set_orbit_max(sr * 25.0F)
      .set_far(sr * 100.0F)
      .set_near(sr * 0.01F);
}
//------------------------------------------------------------------------------
auto sky_viewer::_all_resource_count() noexcept -> span_size_t {
    return _backgrounds.all_resource_count() + _cube_maps.all_resource_count();
}
//------------------------------------------------------------------------------
auto sky_viewer::_loaded_resource_count() noexcept -> span_size_t {
    return _backgrounds.loaded_resource_count() +
           _cube_maps.loaded_resource_count();
}
//------------------------------------------------------------------------------
void sky_viewer::_on_loaded() noexcept {
    _load_progress.update_progress(_loaded_resource_count());
    if(_loaded_resource_count() == _all_resource_count()) {
        _load_progress.finish();
    }
}
//------------------------------------------------------------------------------
void sky_viewer::_on_selected() noexcept {
    if(_backgrounds and _cube_maps) {
        _init_camera();
        _backgrounds.set_skybox_unit(_video, _cube_maps.texture_unit(_video));
        _cube_maps.use(_video);
    }
}
//------------------------------------------------------------------------------
sky_viewer::sky_viewer(execution_context& ctx, video_context& video)
  : common_application{ctx}
  , _video{video}
  , _backgrounds{ctx, video}
  , _cube_maps{ctx, video}
  , _load_progress{ctx.progress(), "loading resources", _all_resource_count()} {
    _backgrounds.loaded.connect(_load_handler());
    _backgrounds.selected.connect(_select_handler());
    _cube_maps.loaded.connect(_load_handler());
    _cube_maps.selected.connect(_select_handler());

    _init_camera();
    _init_inputs();
}
//------------------------------------------------------------------------------
void sky_viewer::_show_settings(const input& i) noexcept {
    if(not i) {
        _show_setting_window = true;
    }
}
//------------------------------------------------------------------------------
auto sky_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void sky_viewer::_clear_background() noexcept {
    _cube_maps.use(_video);
    _backgrounds.clear(_video, _camera);
}
//------------------------------------------------------------------------------
void sky_viewer::_clear_background_default() noexcept {
    _backgrounds.clear_default(_video, _camera);
}
//------------------------------------------------------------------------------
void sky_viewer::_setting_window(const guiplus::imgui_api& gui) noexcept {
    const auto height{
      _backgrounds.settings_height() + _cube_maps.settings_height() + 85.F};
    gui.set_next_window_size({350, height});
    if(gui.begin("Settings", _show_setting_window).or_false()) {
        if(gui.slider_float("FOV", _fov, 20.F, 120.F)) {
            _camera.set_fov(degrees_(_fov));
        }
        gui.same_line();
        gui.help_marker("changes the field of view of the camera");

        _backgrounds.settings("Backgrounds", gui);
        _cube_maps.settings("Skybox", gui);

        gui.separator();
        if(gui.button("Close").or_true()) {
            _show_setting_window = false;
        }
        gui.same_line();
        gui.help_marker("closes this settings window");
        gui.end();
    }
}
//------------------------------------------------------------------------------
void sky_viewer::update_overlays(guiplus::gui_utils& utils) noexcept {
    if(_show_setting_window) {
        _setting_window(utils.imgui);
    }
}
//------------------------------------------------------------------------------
void sky_viewer::update() noexcept {

    _cube_maps.update();
    _backgrounds.update();

    if(_backgrounds and _cube_maps) {
        _clear_background();
    } else {
        _clear_background_default();
        _cube_maps.load_if_needed(context(), _video);
        _backgrounds.load_if_needed(context(), _video);
    }

    auto& state = context().state();

    if(state.user_idle_too_long()) {
        const auto sec{state.frame_time().value()};
        _camera.update_orbit(state.frame_duration().value() * 0.02F)
          .set_azimuth(radians_(sec * 0.1F))
          .set_elevation(radians_(-math::sine_wave01(sec * 0.02F)));
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void sky_viewer::clean_up() noexcept {
    _cube_maps.clean_up(context(), _video);
    _backgrounds.clean_up(context(), _video);
    _video.end();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
