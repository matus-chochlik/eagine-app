/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer:programs;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import :program;

namespace eagine::app {
//------------------------------------------------------------------------------
export class model_viewer_programs
  : public model_viewer_resources<model_viewer_program> {
public:
    model_viewer_programs(execution_context&, video_context&);

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> model_viewer_programs&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> model_viewer_programs&;

    auto set_cube_map_unit(video_context&, oglplus::texture_unit)
      -> model_viewer_programs&;

    auto set_texture_unit(video_context&, oglplus::texture_unit)
      -> model_viewer_programs&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
