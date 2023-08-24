/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_MAIN_HPP
#define EAGINE_APP_MODEL_VIEWER_MAIN_HPP

#include "background.hpp"
#include "geometry.hpp"
#include "program.hpp"

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer : public common_application {

public:
    model_viewer(execution_context&, video_context&);

    auto is_done() noexcept -> bool final;
    void clear_background() noexcept;
    void view_model() noexcept;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    auto _initial_background() -> model_viewer_background_holder;
    auto _initial_geometry() -> model_viewer_geometry_holder;
    auto _initial_program() -> model_viewer_program_holder;
    void _init_camera(const oglplus::sphere bs);

    void _on_loaded(model_viewer_resource_intf&) noexcept;
    auto _load_handler() noexcept {
        return make_callable_ref<&model_viewer::_on_loaded>(this);
    }

    video_context& _video;
    model_viewer_background _background;
    model_viewer_geometry _geometry;
    model_viewer_program _program;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
