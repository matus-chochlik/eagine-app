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
#include "gpu_program.hpp"

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
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    auto _initial_program(execution_context&, video_context&)
      -> unique_holder<model_viewer_program_intf>;

    video_context& _video;
    model_viewer_program _prog;

    orbiting_camera camera;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
