/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_PROGRAMS_HPP
#define EAGINE_APP_MODEL_VIEWER_PROGRAMS_HPP
#include "program.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer_programs
  : public model_viewer_resources<model_viewer_program> {
public:
    model_viewer_programs(execution_context&, video_context&);

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> model_viewer_programs&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> model_viewer_programs&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
