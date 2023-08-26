/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "program.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_program::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> model_viewer_program& {
    assert(_impl);
    _impl->apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_program::set_camera(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_program& {
    assert(_impl);
    _impl->set_camera(video, camera.matrix(video));
    return *this;
}
//------------------------------------------------------------------------------
//  Program resource
//------------------------------------------------------------------------------
class model_viewer_program_resource
  : public model_viewer_program_intf
  , public gl_program_resource {
public:
    model_viewer_program_resource(
      url locator,
      execution_context&,
      video_context&);
    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    void apply_bindings(video_context&, const oglplus::vertex_attrib_bindings&)
      final;
    void set_camera(video_context&, const mat4&) final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
model_viewer_program_resource::model_viewer_program_resource(
  url locator,
  execution_context& ctx,
  video_context&)
  : gl_program_resource{std::move(locator), ctx} {
    gl_program_resource::loaded.connect(
      make_callable_ref<&model_viewer_program_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> _camera_loc;
    signal_loaded();
}
//------------------------------------------------------------------------------
auto model_viewer_program_resource::is_loaded() noexcept -> bool {
    return gl_program_resource::is_loaded();
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::load_if_needed(
  execution_context& ctx,
  video_context&) {
    gl_program_resource::load_if_needed(ctx);
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::use(video_context& video) {
    gl_program_resource::use(video);
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings) {
    gl_program_resource::apply_input_bindings(video, attrib_bindings);
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::set_camera(
  video_context& video,
  const mat4& m) {
    set(video, _camera_loc, m);
}
//------------------------------------------------------------------------------
void model_viewer_program_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_program_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_program>,
  url locator,
  execution_context& ctx,
  video_context& video) -> model_viewer_program_holder {
    return {
      hold<model_viewer_program_resource>, std::move(locator), ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
