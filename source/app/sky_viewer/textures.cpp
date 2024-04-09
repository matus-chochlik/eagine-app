/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.sky_viewer:textures;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :texture;

namespace eagine::app {
//------------------------------------------------------------------------------
export class sky_viewer_cube_maps
  : public sky_viewer_resources<sky_viewer_texture> {
    using base = sky_viewer_resources<sky_viewer_texture>;

public:
    sky_viewer_cube_maps(execution_context&, video_context&);

    auto load_default(const url&) noexcept -> sky_viewer_cube_maps&;

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
