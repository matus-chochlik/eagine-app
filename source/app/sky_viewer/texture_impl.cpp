/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.sky_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto sky_viewer_texture::texture_unit(video_context& video)
  -> oglplus::texture_unit {
    assert(_impl);
    return _impl->texture_unit(video);
}
//------------------------------------------------------------------------------
//  Texture resource
//------------------------------------------------------------------------------
class sky_viewer_texture_resource
  : public sky_viewer_texture_intf
  , public gl_texture_resource {
public:
    sky_viewer_texture_resource(
      const resource_request_params& params,
      execution_context&,
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit);
    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void request_update(const url&, execution_context&, video_context&) final;
    void use(video_context&) final;
    auto texture_unit(video_context&) -> oglplus::texture_unit final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const loaded_resource_base&) noexcept;
    oglplus::texture_target _tex_target;
    oglplus::texture_unit _tex_unit{0};
    bool _was_loaded{false};
};
//------------------------------------------------------------------------------
sky_viewer_texture_resource::sky_viewer_texture_resource(
  const resource_request_params& params,
  execution_context& ctx,
  video_context&,
  oglplus::texture_target tex_target,
  oglplus::texture_unit tex_unit)
  : gl_texture_resource{params, ctx}
  , _tex_target{tex_target}
  , _tex_unit{tex_unit} {
    gl_texture_resource::load_event.connect(
      make_callable_ref<&sky_viewer_texture_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void sky_viewer_texture_resource::_on_loaded(
  const loaded_resource_base& info) noexcept {
    _was_loaded = info.is_loaded();
    if(_was_loaded) {
        signal_loaded();
    } else {
        signal_failed();
    }
}
//------------------------------------------------------------------------------
auto sky_viewer_texture_resource::is_loaded() noexcept -> bool {
    return _was_loaded;
}
//------------------------------------------------------------------------------
void sky_viewer_texture_resource::load_if_needed(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        gl_texture_resource::load_if_needed(
          ctx, _tex_target, texture_unit(video));
    });
}
//------------------------------------------------------------------------------
void sky_viewer_texture_resource::request_update(
  const url& locator,
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        gl_texture_resource::request_update(
          locator,
          ctx.old_loader(),
          ctx.resource_context(),
          _tex_target,
          texture_unit(video));
    });
}
//------------------------------------------------------------------------------
void sky_viewer_texture_resource::use(video_context&) {}
//------------------------------------------------------------------------------
auto sky_viewer_texture_resource::texture_unit(video_context&)
  -> oglplus::texture_unit {
    return _tex_unit;
}
//------------------------------------------------------------------------------
void sky_viewer_texture_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_texture_resource::clean_up(ctx);
}
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<sky_viewer_texture>,
  url locator,
  execution_context& ctx,
  video_context& video,
  oglplus::texture_target tex_target,
  oglplus::texture_unit tex_unit) -> sky_viewer_texture_holder {
    return {
      hold<sky_viewer_texture_resource>,
      resource_request_params{
        .locator = std::move(locator), .max_time = std::chrono::hours{12}},
      ctx,
      video,
      tex_target,
      tex_unit};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
