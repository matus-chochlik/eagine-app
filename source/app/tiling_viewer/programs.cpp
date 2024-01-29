/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.tiling_viewer:programs;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import :program;

namespace eagine::app {
//------------------------------------------------------------------------------
export class tiling_viewer_programs
  : public tiling_viewer_resources<tiling_viewer_program> {
public:
    tiling_viewer_programs(execution_context&, video_context&);

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> tiling_viewer_programs&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> tiling_viewer_programs&;

    auto set_tiling_unit(video_context&, oglplus::texture_unit)
      -> tiling_viewer_programs&;

    auto set_transition_unit(video_context&, oglplus::texture_unit)
      -> tiling_viewer_programs&;

    auto set_tileset_unit(video_context&, oglplus::texture_unit)
      -> tiling_viewer_programs&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
