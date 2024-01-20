/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.tiling_viewer:textures;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :texture;

namespace eagine::app {
//------------------------------------------------------------------------------
export class tiling_viewer_tilings
  : public tiling_viewer_resources<tiling_viewer_texture> {
public:
    tiling_viewer_tilings(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
export class tiling_viewer_transitions
  : public tiling_viewer_resources<tiling_viewer_texture> {
public:
    tiling_viewer_transitions(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
export class tiling_viewer_tilesets
  : public tiling_viewer_resources<tiling_viewer_texture> {
public:
    tiling_viewer_tilesets(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
