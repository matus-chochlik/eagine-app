/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.tiling_viewer:texture;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct tiling_viewer_texture_intf : tiling_viewer_resource_intf {
    virtual auto texture_unit(video_context&) -> oglplus::texture_unit = 0;
};
using tiling_viewer_texture_holder = unique_holder<tiling_viewer_texture_intf>;
//------------------------------------------------------------------------------
class tiling_viewer_texture
  : public tiling_viewer_resource_wrapper<tiling_viewer_texture_intf> {
    using base = tiling_viewer_resource_wrapper<tiling_viewer_texture_intf>;

public:
    using base::base;

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<tiling_viewer_texture>,
  url,
  execution_context&,
  video_context&,
  oglplus::texture_target,
  oglplus::texture_unit) -> tiling_viewer_texture_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
