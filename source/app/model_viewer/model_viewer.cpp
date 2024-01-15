/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer;

import eagine.core;
import eagine.guiplus;
import eagine.oglplus;
import eagine.app;
export import :resource;
export import :background;
export import :backgrounds;
export import :geometry;
export import :models;
export import :program;
export import :programs;
export import :texture;
export import :textures;

namespace eagine::app {
//------------------------------------------------------------------------------
export class model_viewer : public common_application {

public:
    model_viewer(execution_context&, video_context&);

    auto is_done() noexcept -> bool final;
    void update() noexcept final;
    void update_overlays(guiplus::gui_utils& gui) noexcept final;
    void clean_up() noexcept final;

private:
    void _init_inputs();
    void _init_camera(const oglplus::sphere bs);

    auto _all_resource_count() noexcept -> span_size_t;
    auto _loaded_resource_count() noexcept -> span_size_t;
    void _on_loaded() noexcept;
    auto _load_handler() noexcept;
    void _on_selected() noexcept;
    auto _select_handler() noexcept;
    void _clear_background() noexcept;
    void _view_model() noexcept;
    void _show_settings(const input& i) noexcept;
    auto _show_settings_handler() noexcept;
    void _setting_window(const guiplus::imgui_api&) noexcept;

    video_context& _video;
    model_viewer_backgrounds _backgrounds;
    model_viewer_models _models;
    model_viewer_programs _programs;
    model_viewer_cube_maps _cube_maps;
    model_viewer_textures _textures;
    activity_progress _load_progress;

    orbiting_camera _camera;
    float _fov{70.F};
    bool _show_setting_window{false};
};
//------------------------------------------------------------------------------
export auto establish(main_ctx&) -> unique_holder<launchpad>;
//------------------------------------------------------------------------------
} // namespace eagine::app

