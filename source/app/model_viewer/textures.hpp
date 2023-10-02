/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_TEXTURES_HPP
#define EAGINE_APP_MODEL_VIEWER_TEXTURES_HPP
#include "texture.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer_textures
  : public model_viewer_resources<model_viewer_texture> {
public:
    model_viewer_textures(execution_context&, video_context&);

    auto texture_unit(video_context&) -> oglplus::texture_unit::value_type;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
