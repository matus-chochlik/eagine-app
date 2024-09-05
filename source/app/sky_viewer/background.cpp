/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.sky_viewer:background;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct sky_viewer_background_intf : sky_viewer_resource_intf {
    virtual void set_skybox_unit(video_context&, oglplus::texture_unit) = 0;
    virtual void clear(
      video_context&,
      const mat4&,
      const vec3&,
      const float distance) = 0;
};
using sky_viewer_background_holder = unique_holder<sky_viewer_background_intf>;
//------------------------------------------------------------------------------
class sky_viewer_background
  : public sky_viewer_resource_wrapper<sky_viewer_background_intf> {
    using base = sky_viewer_resource_wrapper<sky_viewer_background_intf>;

public:
    using base::base;

    auto set_skybox_unit(video_context&, oglplus::texture_unit)
      -> sky_viewer_background&;

    auto clear(video_context&, orbiting_camera&) -> sky_viewer_background&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<sky_viewer_background>,
  url,
  execution_context&,
  video_context&) -> sky_viewer_background_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
