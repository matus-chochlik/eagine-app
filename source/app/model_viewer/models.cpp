/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

export module eagine.app.model_viewer:models;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import :geometry;

namespace eagine::app {
//------------------------------------------------------------------------------
export class model_viewer_models
  : public model_viewer_resources<model_viewer_geometry> {
public:
    model_viewer_models(execution_context&, video_context&);

    auto bounding_sphere() noexcept -> oglplus::sphere;
    auto attrib_bindings() noexcept -> const oglplus::vertex_attrib_bindings&;
    auto draw(video_context&) -> model_viewer_models&;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
