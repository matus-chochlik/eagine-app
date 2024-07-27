/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.sky_viewer;

import std;
import eagine.core;
import eagine.guiplus;
import eagine.oglplus;
import eagine.app;
export import :resource;
export import :background;
export import :backgrounds;
export import :texture;
export import :textures;

namespace eagine::app {
//------------------------------------------------------------------------------
export class sky_viewer : public common_application {

public:
    sky_viewer(execution_context&, video_context&);

    auto is_done() noexcept -> bool final;
    auto should_dump_frame() noexcept -> bool final;
    void update() noexcept final;
    void update_overlays(guiplus::gui_utils& gui) noexcept final;
    void clean_up() noexcept final;

private:
    auto _get_resolution() noexcept -> int;
    auto _get_animation_mode() noexcept -> bool;
    auto _get_animation_template() noexcept -> std::string;

    void _init_inputs();
    void _init_camera();

    void _on_cube_map_loaded() noexcept;
    void _on_cube_map_failed() noexcept;
    auto _cube_map_load_handler() noexcept;
    auto _cube_map_fail_handler() noexcept;
    void _on_selected() noexcept;
    auto _select_handler() noexcept;
    void _update_camera() noexcept;
    void _clear_background() noexcept;
    void _clear_background_default() noexcept;
    void _show_settings(const input& i) noexcept;
    auto _show_settings_handler() noexcept;
    void _setting_window(const guiplus::imgui_api&) noexcept;

    auto _make_anim_url(long frame_no) noexcept -> url;
    auto _make_anim_url() noexcept -> url;

    video_context& _video;

    orbiting_camera _camera;

    float _fov{90.F};
    long _anim_frame_no{0};
    const std::string _animation_template{_get_animation_template()};
    const int _resolution{_get_resolution()};
    const bool _animation_mode{_get_animation_mode()};
    bool _anim_frame_ready{false};
    bool _show_setting_window{false};

    sky_viewer_backgrounds _backgrounds;
    sky_viewer_cube_maps _cube_maps;
};
//------------------------------------------------------------------------------
export auto establish(main_ctx&) -> unique_holder<launchpad>;
//------------------------------------------------------------------------------
} // namespace eagine::app

