/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_GEOMETRY_HPP
#define EAGINE_APP_MODEL_VIEWER_GEOMETRY_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_geometry_intf : interface<model_viewer_geometry_intf> {
    virtual void use(video_context&) = 0;

    virtual auto bounding_sphere() noexcept -> oglplus::sphere = 0;
    virtual void clean_up(execution_context&, video_context&) = 0;
};
//------------------------------------------------------------------------------
class model_viewer_geometry {
public:
    model_viewer_geometry(
      unique_holder<model_viewer_geometry_intf> impl) noexcept
      : _impl{std::move(impl)} {}

    explicit operator bool() const noexcept {
        return bool(_impl);
    }

    auto use(video_context&) -> model_viewer_geometry&;

    auto bounding_sphere() noexcept -> oglplus::sphere;

    auto clean_up(execution_context&, video_context&) -> model_viewer_geometry&;

private:
    unique_holder<model_viewer_geometry_intf> _impl;
};
//------------------------------------------------------------------------------
auto make_default_geometry(execution_context&, video_context&)
  -> unique_holder<model_viewer_geometry_intf>;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
