/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.sky_viewer:texture;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct sky_viewer_texture_intf : sky_viewer_resource_intf {
    virtual auto texture_unit(video_context&) -> oglplus::texture_unit = 0;
};
using sky_viewer_texture_holder = unique_holder<sky_viewer_texture_intf>;
//------------------------------------------------------------------------------
class sky_viewer_texture
  : public sky_viewer_resource_wrapper<sky_viewer_texture_intf> {
    using base = sky_viewer_resource_wrapper<sky_viewer_texture_intf>;

public:
    using base::base;

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<sky_viewer_texture>,
  url,
  execution_context&,
  video_context&,
  oglplus::texture_target,
  oglplus::texture_unit) -> sky_viewer_texture_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
