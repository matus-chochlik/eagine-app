/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.tiling_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto tiling_viewer_program::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> tiling_viewer_program& {
    assert(_impl);
    _impl->apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_program::set_camera(
  video_context& video,
  orbiting_camera& camera) -> tiling_viewer_program& {
    assert(_impl);
    _impl->set_camera(video, camera.matrix(video));
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_program::set_tiling_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) -> tiling_viewer_program& {
    assert(_impl);
    _impl->set_tiling_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
auto tiling_viewer_program::set_tileset_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) -> tiling_viewer_program& {
    assert(_impl);
    _impl->set_tileset_unit(video, tu);
    return *this;
}
//------------------------------------------------------------------------------
//  Program resource
//------------------------------------------------------------------------------
class tiling_viewer_program_resource
  : public tiling_viewer_program_intf
  , public gl_program_resource {
public:
    tiling_viewer_program_resource(
      url locator,
      execution_context&,
      video_context&);
    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    void apply_bindings(video_context&, const oglplus::vertex_attrib_bindings&)
      final;
    void set_camera(video_context&, const mat4&) final;
    void set_tiling_unit(video_context&, oglplus::texture_unit::value_type)
      final;
    void set_tileset_unit(video_context&, oglplus::texture_unit::value_type)
      final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location _camera_loc{};
    oglplus::uniform_location _tiling_loc{};
    oglplus::uniform_location _tileset_loc{};
};
//------------------------------------------------------------------------------
tiling_viewer_program_resource::tiling_viewer_program_resource(
  url locator,
  execution_context& ctx,
  video_context&)
  : gl_program_resource{std::move(locator), ctx} {
    gl_program_resource::loaded.connect(
      make_callable_ref<&tiling_viewer_program_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera").and_then(_1.assign_to(_camera_loc));
    info.get_uniform_location("Tiling").and_then(_1.assign_to(_tiling_loc));
    info.get_uniform_location("Texture0").and_then(_1.assign_to(_tileset_loc));
    signal_loaded();
}
//------------------------------------------------------------------------------
auto tiling_viewer_program_resource::is_loaded() noexcept -> bool {
    return gl_program_resource::is_loaded();
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::load_if_needed(
  execution_context& ctx,
  video_context&) {
    gl_program_resource::load_if_needed(ctx);
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::use(video_context& video) {
    gl_program_resource::use(video);
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings) {
    gl_program_resource::apply_input_bindings(video, attrib_bindings);
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::set_camera(
  video_context& video,
  const mat4& m) {
    set(video, _camera_loc, m);
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::set_tiling_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) {
    set(video, _tiling_loc, int(tu));
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::set_tileset_unit(
  video_context& video,
  oglplus::texture_unit::value_type tu) {
    set(video, _tileset_loc, int(tu));
}
//------------------------------------------------------------------------------
void tiling_viewer_program_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_program_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<tiling_viewer_program>,
  url locator,
  execution_context& ctx,
  video_context& video) -> tiling_viewer_program_holder {
    return {
      hold<tiling_viewer_program_resource>, std::move(locator), ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
