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
auto sky_viewer::_cube_map_load_handler() noexcept {
    return make_callable_ref<&sky_viewer::_on_cube_map_loaded>(this);
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
auto sky_viewer::_make_anim_url(long frame_no) noexcept -> url {
    std::string loc;
    loc.append("eagitex:///cube_map_sky");
    loc.append("?size=");
    loc.append(std::to_string(_resolution.value_or(256)));
    loc.append("&sun_elevation_deg=");
    loc.append(std::to_string(0.125F * frame_no - 5.F));
    loc.append("&sun_azimuth_deg=");
    loc.append(std::to_string(0.25F * frame_no + 180.F));
    loc.append("&cloud_offset_x=");
    loc.append(std::to_string(0.0005F * frame_no));
    loc.append("&cloud_offset_y=");
    loc.append(std::to_string(0.0001F * frame_no));
    loc.append("&cloudiness_ratio=");
    loc.append(std::to_string(math::clamp(0.00125F * frame_no, 0.F, 1.F)));

    return url{std::move(loc)};
}
//------------------------------------------------------------------------------
void sky_viewer::_on_cube_map_loaded() noexcept {
    if(_animation_mode) {
        ++_anim_frame_no;
        _cube_maps.update_default(
          context(), _video, _make_anim_url(_anim_frame_no));
        _anim_frame_ready = true;
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
auto sky_viewer::_get_animation_mode() noexcept -> bool {
    return context().main_context().args().find("--animation");
}
//------------------------------------------------------------------------------
sky_viewer::sky_viewer(execution_context& ctx, video_context& video)
  : common_application{ctx}
  , _video{video}
  , _backgrounds{ctx, video}
  , _cube_maps{ctx, video, _make_anim_url(0)} {
    _backgrounds.selected.connect(_select_handler());
    _cube_maps.loaded.connect(_cube_map_load_handler());
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
auto sky_viewer::should_dump_frame() noexcept -> bool {
    if(_animation_mode) {
        if(_anim_frame_ready) {
            _anim_frame_ready = false;
        } else {
            return false;
        }
    }
    return true;
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
void sky_viewer::_update_camera() noexcept {
    auto& state = context().state();

    if(_animation_mode) {
        if(_anim_frame_ready) {
            const auto frame_duration{1.F / 30.F};
            const auto sec{float(_anim_frame_no) * frame_duration};
            _camera.update_orbit(frame_duration * 0.02F)
              .set_azimuth(radians_(sec * 0.1F))
              .set_elevation(radians_(-math::sine_wave01(sec * 0.02F)));
        }
    } else if(state.user_idle_too_long()) {
        const auto sec{state.frame_time().value()};
        _camera.update_orbit(state.frame_duration().value() * 0.02F)
          .set_azimuth(radians_(sec * 0.1F))
          .set_elevation(radians_(-math::sine_wave01(sec * 0.02F)));
    }
}
//------------------------------------------------------------------------------
void sky_viewer::update() noexcept {
    _update_camera();

    _cube_maps.update();
    _cube_maps.load_if_needed(context(), _video);
    _backgrounds.update();

    if(_backgrounds and _cube_maps) {
        _clear_background();
    } else {
        _clear_background_default();
        _backgrounds.load_if_needed(context(), _video);
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void sky_viewer::clean_up() noexcept {
    _cube_maps.clean_up(context(), _video);
    _backgrounds.clean_up(context(), _video);
    _video.end();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
