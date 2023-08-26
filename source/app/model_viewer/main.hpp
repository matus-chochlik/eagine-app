/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_MAIN_HPP
#define EAGINE_APP_MODEL_VIEWER_MAIN_HPP

#include "backgrounds.hpp"
#include "models.hpp"
#include "programs.hpp"

import eagine.core;
import eagine.oglplus;
import eagine.guiplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer : public common_application {

public:
    model_viewer(execution_context&, video_context&);

    auto is_done() noexcept -> bool final;
    void update() noexcept final;
    void update_overlays(guiplus::gui_utils& gui) noexcept final;
    void clean_up() noexcept final;

private:
    void _init_inputs();
    void _init_camera(const oglplus::sphere bs);

    void _on_loaded() noexcept;
    auto _load_handler() noexcept;
    void _clear_background() noexcept;
    void _view_model() noexcept;
    void _show_settings(const input& i) noexcept;
    auto _show_settings_handler() noexcept;
    void _setting_window(const guiplus::imgui_api&) noexcept;

    video_context& _video;
    model_viewer_backgrounds _backgrounds;
    model_viewer_models _models;
    model_viewer_programs _programs;

    orbiting_camera _camera;
    float _fov{70.F};
    bool _show_setting_window{false};
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
