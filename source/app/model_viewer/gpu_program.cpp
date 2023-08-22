/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "gpu_program.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_program::use(video_context& video) -> model_viewer_program& {
    assert(_impl);
    _impl->use(video);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_program::set_camera(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_program& {
    _impl->set_camera(video, camera.matrix(video));
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_program::clean_up(execution_context& ctx, video_context& video)
  -> model_viewer_program& {
    if(_impl) {
        _impl->clean_up(ctx, video);
    }
    return *this;
}
//------------------------------------------------------------------------------
//  Default program
//------------------------------------------------------------------------------
class default_model_viewer_program
  : public model_viewer_program_intf
  , public gl_program_resource {
public:
    default_model_viewer_program(execution_context&, video_context&);
    void use(video_context&) final;
    void set_camera(video_context&, const mat4&) final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
default_model_viewer_program::default_model_viewer_program(
  execution_context& ctx,
  video_context&)
  : gl_program_resource{url{"json:///DfaultProg"}, ctx} {
    loaded.connect(
      make_callable_ref<&default_model_viewer_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void default_model_viewer_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> _camera_loc;
}
//------------------------------------------------------------------------------
void default_model_viewer_program::use(video_context& video) {
    gl_program_resource::use(video);
}
//------------------------------------------------------------------------------
void default_model_viewer_program::set_camera(
  video_context& video,
  const mat4& m) {
    set(video, _camera_loc, m);
}
//------------------------------------------------------------------------------
void default_model_viewer_program::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_program_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
auto make_default_program(execution_context& ctx, video_context& video)
  -> unique_holder<model_viewer_program_intf> {
    return {hold<default_model_viewer_program>, ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
