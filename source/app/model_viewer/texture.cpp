/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer:texture;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_texture_intf : model_viewer_resource_intf {
    virtual auto texture_unit(video_context&)
      -> oglplus::texture_unit::value_type = 0;
};
using model_viewer_texture_holder = unique_holder<model_viewer_texture_intf>;
//------------------------------------------------------------------------------
class model_viewer_texture
  : public model_viewer_resource_wrapper<model_viewer_texture_intf> {
    using base = model_viewer_resource_wrapper<model_viewer_texture_intf>;

public:
    using base::base;

    auto texture_unit(video_context&) -> oglplus::texture_unit::value_type;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_texture>,
  url,
  execution_context&,
  video_context&,
  oglplus::texture_unit::value_type) -> model_viewer_texture_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
