/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_PROGRAM_HPP
#define EAGINE_APP_MODEL_VIEWER_PROGRAM_HPP
#include "resource.hpp"

import eagine.oglplus;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_program_intf : model_viewer_resource_intf {
    virtual void apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings&) = 0;
    virtual void set_camera(video_context&, const mat4&) = 0;
    virtual void set_texture_unit(
      video_context&,
      oglplus::texture_unit::value_type) = 0;
};
using model_viewer_program_holder = unique_holder<model_viewer_program_intf>;
//------------------------------------------------------------------------------
class model_viewer_program
  : public model_viewer_resource_wrapper<model_viewer_program_intf> {
    using base = model_viewer_resource_wrapper<model_viewer_program_intf>;

public:
    using base::base;

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> model_viewer_program&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> model_viewer_program&;

    auto set_texture_unit(video_context&, oglplus::texture_unit::value_type)
      -> model_viewer_program&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_program>,
  url,
  execution_context&,
  video_context&) -> model_viewer_program_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
