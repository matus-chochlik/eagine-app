/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_GEOMETRY_HPP
#define EAGINE_APP_MODEL_VIEWER_GEOMETRY_HPP
#include "resource.hpp"

import eagine.shapes;
import eagine.oglplus;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_geometry_intf : model_viewer_resource_intf {
    virtual void draw(video_context&) = 0;
    virtual auto bounding_sphere() noexcept -> oglplus::sphere = 0;
    virtual auto attrib_bindings() noexcept
      -> const oglplus::vertex_attrib_bindings& = 0;
};
using model_viewer_geometry_holder = unique_holder<model_viewer_geometry_intf>;
//------------------------------------------------------------------------------
class model_viewer_geometry
  : public model_viewer_resource_wrapper<model_viewer_geometry_intf> {
    using base = model_viewer_resource_wrapper<model_viewer_geometry_intf>;

public:
    using base::base;

    auto bounding_sphere() noexcept -> oglplus::sphere;
    auto attrib_bindings() noexcept -> const oglplus::vertex_attrib_bindings&;
    auto draw(video_context&) -> model_viewer_geometry&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_geometry>,
  url,
  execution_context&,
  video_context&) -> model_viewer_geometry_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
