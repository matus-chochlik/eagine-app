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
auto tiling_viewer::_load_handler() noexcept {
    return make_callable_ref<&tiling_viewer::_on_loaded>(this);
}
//------------------------------------------------------------------------------
auto tiling_viewer::_select_handler() noexcept {
    return make_callable_ref<&tiling_viewer::_on_selected>(this);
}
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
      .set_far(sr * 50.0F)
      .set_orbit_min(sr * 0.51F)
      .set_orbit_max(sr * 7.5F);
}
//------------------------------------------------------------------------------
auto tiling_viewer::_all_resource_count() noexcept -> span_size_t {
    return _models.all_resource_count() + _programs.all_resource_count() +
           _tilings.all_resource_count() + _tilesets.all_resource_count();
}
//------------------------------------------------------------------------------
auto tiling_viewer::_loaded_resource_count() noexcept -> span_size_t {
    return _models.loaded_resource_count() + _programs.loaded_resource_count() +
           _tilings.loaded_resource_count() + _tilesets.loaded_resource_count();
}
//------------------------------------------------------------------------------
void tiling_viewer::_on_loaded() noexcept {
    _load_progress.update_progress(_loaded_resource_count());
    if(_loaded_resource_count() == _all_resource_count()) {
        _load_progress.finish();
    }
}
//------------------------------------------------------------------------------
void tiling_viewer::_on_selected() noexcept {
    if(_models and _programs) {
        _init_camera(_models.bounding_sphere());
        _programs.use(_video);
        _programs.apply_bindings(_video, _models.attrib_bindings());
        _programs.set_tiling_unit(_video, _tilings.texture_unit(_video));
        _programs.set_tileset_unit(_video, _tilesets.texture_unit(_video));
    }
}
//------------------------------------------------------------------------------
tiling_viewer::tiling_viewer(execution_context& ctx, video_context& video)
  : common_application{ctx}
  , _video{video}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.0F}, {0.4F, 0.4F, 0.4F, 0.0F}, 1.F}
  , _models{ctx, video}
  , _programs{ctx, video}
  , _tilings{ctx, video}
  , _tilesets{ctx, video}
  , _load_progress{ctx.progress(), "loading resources", _all_resource_count()} {
    _models.loaded.connect(_load_handler());
    _models.selected.connect(_select_handler());
    _programs.loaded.connect(_load_handler());
    _programs.selected.connect(_select_handler());
    _tilings.loaded.connect(_load_handler());
    _tilings.selected.connect(_select_handler());
    _tilesets.loaded.connect(_load_handler());
    _tilesets.selected.connect(_select_handler());

    _init_camera({{0.F, 0.F, 0.F}, 1.F});
    _init_inputs();
}
//------------------------------------------------------------------------------
void tiling_viewer::_view_tiling() noexcept {
    _programs.use(_video);
    _programs.set_camera(_video, _camera);

    _tilings.use(_video);
    _tilesets.use(_video);
    _models.use(_video);
    _models.draw(_video);
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
    const auto height{
      _tilings.settings_height() + _tilesets.settings_height() +
      _programs.settings_height() + _models.settings_height() + 85.F};
    gui.set_next_window_size({350, height});
    if(gui.begin("Settings", _show_setting_window).or_false()) {
        if(gui.slider_float("FOV", _fov, 20.F, 120.F)) {
            _camera.set_fov(degrees_(_fov));
        }
        gui.same_line();
        gui.help_marker("changes the field of view of the camera");

        _tilings.settings("Tilings", gui);
        _tilesets.settings("Tilesets", gui);
        _programs.settings("Programs", gui);
        _models.settings("Models", gui);

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
void tiling_viewer::update_overlays(guiplus::gui_utils& utils) noexcept {
    if(_show_setting_window) {
        _setting_window(utils.imgui);
    }
}
//------------------------------------------------------------------------------
void tiling_viewer::update() noexcept {
    _bg.clear(_video, _camera);

    _models.update();
    _programs.update();
    _tilings.update();
    _tilesets.update();

    if(_models and _programs and _tilings and _tilesets) {
        auto& state = context().state();
        if(state.user_idle_too_long()) {
            _camera.idle_update(state);
        }
        _view_tiling();
    } else {
        _models.load_if_needed(context(), _video);
        _programs.load_if_needed(context(), _video);
        _tilings.load_if_needed(context(), _video);
        _tilesets.load_if_needed(context(), _video);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void tiling_viewer::clean_up() noexcept {
    _tilesets.clean_up(context(), _video);
    _tilings.clean_up(context(), _video);
    _programs.clean_up(context(), _video);
    _models.clean_up(context(), _video);
    _bg.clean_up(_video);
    _video.end();
}
//------------------------------------------------------------------------------
} // namespace eagine::app

