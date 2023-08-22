/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_GPU_PROGRAM_HPP
#define EAGINE_APP_MODEL_VIEWER_GPU_PROGRAM_HPP

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_program_intf : interface<model_viewer_program_intf> {
    virtual void use(video_context&) = 0;
    virtual void set_camera(video_context&, const mat4&) = 0;
    virtual void clean_up(execution_context&, video_context&) = 0;
};
//------------------------------------------------------------------------------
class model_viewer_program {
public:
    model_viewer_program(unique_holder<model_viewer_program_intf> impl) noexcept
      : _impl{std::move(impl)} {}

    explicit operator bool() const noexcept {
        return bool(_impl);
    }

    auto use(video_context&) -> model_viewer_program&;

    auto set_camera(video_context&, orbiting_camera& projection)
      -> model_viewer_program&;

    auto clean_up(execution_context&, video_context&) -> model_viewer_program&;

private:
    unique_holder<model_viewer_program_intf> _impl;
};
//------------------------------------------------------------------------------
auto make_default_program(execution_context&, video_context&)
  -> unique_holder<model_viewer_program_intf>;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
