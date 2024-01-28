/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.tiling_viewer:geometry;

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct tiling_viewer_geometry_intf : tiling_viewer_resource_intf {
    virtual void draw(video_context&) = 0;
    virtual auto bounding_sphere() noexcept -> oglplus::sphere = 0;
    virtual auto attrib_bindings() noexcept
      -> const oglplus::vertex_attrib_bindings& = 0;
};
using tiling_viewer_geometry_holder =
  unique_holder<tiling_viewer_geometry_intf>;
//------------------------------------------------------------------------------
class tiling_viewer_geometry
  : public tiling_viewer_resource_wrapper<tiling_viewer_geometry_intf> {
    using base = tiling_viewer_resource_wrapper<tiling_viewer_geometry_intf>;

public:
    using base::base;

    auto bounding_sphere() noexcept -> oglplus::sphere;
    auto attrib_bindings() noexcept -> const oglplus::vertex_attrib_bindings&;
    auto draw(video_context&) -> tiling_viewer_geometry&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<tiling_viewer_geometry>,
  url,
  execution_context&,
  video_context&) -> tiling_viewer_geometry_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
