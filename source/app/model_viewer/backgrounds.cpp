/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer:backgrounds;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import :background;

namespace eagine::app {
//------------------------------------------------------------------------------
export class model_viewer_backgrounds
  : public model_viewer_resources<model_viewer_background> {
public:
    model_viewer_backgrounds(execution_context&, video_context&);

    auto clear(video_context&, orbiting_camera&) -> model_viewer_backgrounds&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
