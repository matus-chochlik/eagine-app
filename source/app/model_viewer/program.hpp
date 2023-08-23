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
    virtual void set_camera(video_context&, const mat4&) = 0;
};
using model_viewer_program_holder = unique_holder<model_viewer_program_intf>;
//------------------------------------------------------------------------------
class model_viewer_program
  : public model_viewer_resource_wrapper<model_viewer_program_intf> {
    using base = model_viewer_resource_wrapper<model_viewer_program_intf>;

public:
    using base::base;

    auto set_camera(video_context&, orbiting_camera& projection)
      -> model_viewer_program&;
};
//------------------------------------------------------------------------------
auto make_default_program(execution_context&, video_context&)
  -> model_viewer_program_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
