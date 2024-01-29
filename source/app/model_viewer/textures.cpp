/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.model_viewer:textures;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :texture;

namespace eagine::app {
//------------------------------------------------------------------------------
export class model_viewer_cube_maps
  : public model_viewer_resources<model_viewer_texture> {
public:
    model_viewer_cube_maps(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
export class model_viewer_textures
  : public model_viewer_resources<model_viewer_texture> {
public:
    model_viewer_textures(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
