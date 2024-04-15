/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.sky_viewer:backgrounds;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import :background;

namespace eagine::app {
//------------------------------------------------------------------------------
export class sky_viewer_backgrounds
  : public sky_viewer_resources<sky_viewer_background> {
public:
    sky_viewer_backgrounds(execution_context&, video_context&);

    auto set_skybox_unit(video_context&, oglplus::texture_unit)
      -> sky_viewer_backgrounds&;

    auto clear(video_context&, orbiting_camera&) -> sky_viewer_backgrounds&;
    auto clear_default(video_context&, orbiting_camera&)
      -> sky_viewer_backgrounds&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
