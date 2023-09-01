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
auto model_viewer::_load_handler() noexcept {
    return make_callable_ref<&model_viewer::_on_loaded>(this);
}
//------------------------------------------------------------------------------
auto model_viewer::_select_handler() noexcept {
    return make_callable_ref<&model_viewer::_on_selected>(this);
}
//------------------------------------------------------------------------------
auto model_viewer::_show_settings_handler() noexcept {
    return make_callable_ref<&model_viewer::_show_settings>(this);
}
//------------------------------------------------------------------------------
void model_viewer::_init_inputs() {
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
void model_viewer::_init_camera(const oglplus::sphere bs) {
    const auto sr{bs.radius()};
    _camera.set_fov(degrees_(_fov))
      .set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 100.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 4.0F);
}
//------------------------------------------------------------------------------
auto model_viewer::_all_resource_count() noexcept -> span_size_t {
    return _backgrounds.all_resource_count() + _models.all_resource_count() +
           _programs.all_resource_count() + _textures.all_resource_count();
}
//------------------------------------------------------------------------------
auto model_viewer::_loaded_resource_count() noexcept -> span_size_t {
    return _backgrounds.loaded_resource_count() +
           _models.loaded_resource_count() + _programs.loaded_resource_count() +
           _textures.loaded_resource_count();
}
//------------------------------------------------------------------------------
void model_viewer::_on_loaded() noexcept {
    _load_progress.update_progress(_loaded_resource_count());
    if(_loaded_resource_count() == _all_resource_count()) {
        _load_progress.finish();
    }
}
//------------------------------------------------------------------------------
void model_viewer::_on_selected() noexcept {
    if(_backgrounds and _models and _programs and _textures) {
        _init_camera(_models.bounding_sphere());
        _programs.use(_video);
        _programs.apply_bindings(_video, _models.attrib_bindings());
    }
}
//------------------------------------------------------------------------------
model_viewer::model_viewer(execution_context& ctx, video_context& video)
  : common_application{ctx}
  , _video{video}
  , _backgrounds{ctx, video}
  , _models{ctx, video}
  , _programs{ctx, video}
  , _textures{ctx, video}
  , _load_progress{ctx.progress(), "Loading resources", _all_resource_count()} {
    _backgrounds.loaded.connect(_load_handler());
    _backgrounds.selected.connect(_select_handler());
    _models.loaded.connect(_load_handler());
    _models.selected.connect(_select_handler());
    _programs.loaded.connect(_load_handler());
    _programs.selected.connect(_select_handler());
    _textures.loaded.connect(_load_handler());
    _textures.selected.connect(_select_handler());

    _init_camera({{0.F, 0.F, 0.F}, 1.F});
    _init_inputs();
}
//------------------------------------------------------------------------------
void model_viewer::_show_settings(const input& i) noexcept {
    if(not i) {
        _show_setting_window = true;
    }
}
//------------------------------------------------------------------------------
auto model_viewer::is_done() noexcept -> bool {
    return false;
}
//------------------------------------------------------------------------------
void model_viewer::_clear_background() noexcept {
    _backgrounds.use(_video);
    _backgrounds.clear(_video, _camera);
}
//------------------------------------------------------------------------------
void model_viewer::_view_model() noexcept {
    _programs.use(_video);
    _programs.set_camera(_video, _camera);

    _textures.use(_video);
    _models.use(_video);
    _models.draw(_video);
}
//------------------------------------------------------------------------------
void model_viewer::_setting_window(const guiplus::imgui_api& gui) noexcept {
    gui.set_next_window_size({350, 240});
    if(gui.begin("Settings", _show_setting_window).or_false()) {
        if(gui.slider_float("FOV", _fov, 20.F, 120.F)) {
            _camera.set_fov(degrees_(_fov));
        }
        gui.same_line();
        gui.help_marker("changes the field of view of the camera");

        _backgrounds.settings("Backgrounds", gui);
        _models.settings("Models", gui);
        _programs.settings("Programs", gui);
        _textures.settings("Textures", gui);

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
void model_viewer::update_overlays(guiplus::gui_utils& utils) noexcept {
    if(_show_setting_window) {
        _setting_window(utils.imgui);
    }
}
//------------------------------------------------------------------------------
void model_viewer::update() noexcept {
    auto& state = context().state();
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    _backgrounds.update();
    if(_backgrounds) {
        _clear_background();
    } else {
        _backgrounds.load_if_needed(context(), _video);
    }

    _models.update();
    _programs.update();
    _textures.update();
    if(_models and _programs and _textures) {
        _view_model();
    } else {
        _models.load_if_needed(context(), _video);
        _programs.load_if_needed(context(), _video);
        _textures.load_if_needed(context(), _video);
    }
    _video.commit();
}
//------------------------------------------------------------------------------
void model_viewer::clean_up() noexcept {
    _textures.clean_up(context(), _video);
    _programs.clean_up(context(), _video);
    _models.clean_up(context(), _video);
    _backgrounds.clean_up(context(), _video);
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
//------------------------------------------------------------------------------
