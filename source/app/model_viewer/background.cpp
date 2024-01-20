/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer:background;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_background_intf : model_viewer_resource_intf {
    virtual void set_skybox_unit(video_context&, oglplus::texture_unit) = 0;
    virtual void clear(video_context&, const mat4&, const float distance) = 0;
};
using model_viewer_background_holder =
  unique_holder<model_viewer_background_intf>;
//------------------------------------------------------------------------------
class model_viewer_background
  : public model_viewer_resource_wrapper<model_viewer_background_intf> {
    using base = model_viewer_resource_wrapper<model_viewer_background_intf>;

public:
    using base::base;

    auto set_skybox_unit(video_context&, oglplus::texture_unit)
      -> model_viewer_background&;

    auto clear(video_context&, orbiting_camera&) -> model_viewer_background&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_background>,
  url,
  execution_context&,
  video_context&) -> model_viewer_background_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
