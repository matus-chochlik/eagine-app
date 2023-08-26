/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "background.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_background::clear(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_background& {
    assert(_impl);
    _impl->clear(video, camera.matrix(video), camera.skybox_distance());
    return *this;
}
//------------------------------------------------------------------------------
//  Background
//------------------------------------------------------------------------------
class model_viewer_default_background : public model_viewer_background_intf {
public:
    model_viewer_default_background(execution_context&, video_context&);

    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    void clear(video_context&, const mat4& camera, const float distance) final;
    void clean_up(execution_context&, video_context&) final;

private:
    background_icosahedron _bg;
};
//------------------------------------------------------------------------------
model_viewer_default_background::model_viewer_default_background(
  execution_context&,
  video_context& video)
  : _bg{video, {0.5F, 0.5F, 0.5F, 1.F}, {0.25F, 0.25F, 0.25F, 1.F}, 1.F} {}
//------------------------------------------------------------------------------
auto model_viewer_default_background::is_loaded() noexcept -> bool {
    return true;
}
//------------------------------------------------------------------------------
void model_viewer_default_background::load_if_needed(
  execution_context&,
  video_context&) {}
//------------------------------------------------------------------------------
void model_viewer_default_background::use(video_context&) {}
//------------------------------------------------------------------------------
void model_viewer_default_background::clear(
  video_context& video,
  const mat4& camera,
  const float distance) {
    _bg.clear(video, camera, distance);
}
//------------------------------------------------------------------------------
void model_viewer_default_background::clean_up(
  execution_context&,
  video_context& video) {
    _bg.clean_up(video);
}
//------------------------------------------------------------------------------
//  Default background
//------------------------------------------------------------------------------
auto make_default_background(execution_context& ctx, video_context& video)
  -> model_viewer_background_holder {
    return {hold<model_viewer_default_background>, ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
