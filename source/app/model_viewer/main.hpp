/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_MAIN_HPP
#define EAGINE_APP_MODEL_VIEWER_MAIN_HPP

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
    void view_model() noexcept;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    auto _initial_geometry() -> model_viewer_geometry_holder;
    auto _initial_program() -> model_viewer_program_holder;

    void on_loaded(model_viewer_resource_intf&) noexcept;

    video_context& _video;
    model_viewer_geometry _geometry;
    model_viewer_program _program;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
