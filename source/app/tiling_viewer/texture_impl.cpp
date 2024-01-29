/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
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
auto tiling_viewer_texture::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    assert(_impl);
    return _impl->texture_unit(video);
}
//------------------------------------------------------------------------------
//  Texture resource
//------------------------------------------------------------------------------
class tiling_viewer_texture_resource
  : public tiling_viewer_texture_intf
  , public gl_texture_resource {
public:
    tiling_viewer_texture_resource(
      url locator,
      execution_context&,
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit);
    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    auto texture_unit(video_context&) -> oglplus::texture_unit final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const gl_texture_resource::load_info&) noexcept;
    oglplus::texture_target _tex_target;
    oglplus::texture_unit _tex_unit{0};
};
//------------------------------------------------------------------------------
tiling_viewer_texture_resource::tiling_viewer_texture_resource(
  url locator,
  execution_context& ctx,
  video_context&,
  oglplus::texture_target tex_target,
  oglplus::texture_unit tex_unit)
  : gl_texture_resource{std::move(locator), ctx}
  , _tex_target{tex_target}
  , _tex_unit{tex_unit} {
    gl_texture_resource::loaded.connect(
      make_callable_ref<&tiling_viewer_texture_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void tiling_viewer_texture_resource::_on_loaded(
  const gl_texture_resource::load_info&) noexcept {
    signal_loaded();
}
//------------------------------------------------------------------------------
auto tiling_viewer_texture_resource::is_loaded() noexcept -> bool {
    return gl_texture_resource::is_loaded();
}
//------------------------------------------------------------------------------
void tiling_viewer_texture_resource::load_if_needed(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        gl_texture_resource::load_if_needed(
          ctx, _tex_target, texture_unit(video));
    });
}
//------------------------------------------------------------------------------
void tiling_viewer_texture_resource::use(video_context&) {}
//------------------------------------------------------------------------------
auto tiling_viewer_texture_resource::texture_unit(video_context&)
  -> oglplus::texture_unit {
    return _tex_unit;
}
//------------------------------------------------------------------------------
void tiling_viewer_texture_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_texture_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<tiling_viewer_texture>,
  url locator,
  execution_context& ctx,
  video_context& video,
  oglplus::texture_target tex_target,
  oglplus::texture_unit tex_unit) -> tiling_viewer_texture_holder {
    return {
      hold<tiling_viewer_texture_resource>,
      std::move(locator),
      ctx,
      video,
      tex_target,
      tex_unit};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
