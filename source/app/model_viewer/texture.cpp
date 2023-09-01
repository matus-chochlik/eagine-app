/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "texture.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_texture::texture_unit(video_context& video)
  -> oglplus::texture_unit::value_type {
    assert(_impl);
    return _impl->texture_unit(video);
}
//------------------------------------------------------------------------------
//  Texture resource
//------------------------------------------------------------------------------
class model_viewer_texture_resource
  : public model_viewer_texture_intf
  , public gl_texture_resource {
public:
    model_viewer_texture_resource(
      url locator,
      execution_context&,
      video_context&);
    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    auto texture_unit(video_context&)
      -> oglplus::texture_unit::value_type final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(const gl_texture_resource::load_info&) noexcept;
};
//------------------------------------------------------------------------------
model_viewer_texture_resource::model_viewer_texture_resource(
  url locator,
  execution_context& ctx,
  video_context&)
  : gl_texture_resource{std::move(locator), ctx} {
    gl_texture_resource::loaded.connect(
      make_callable_ref<&model_viewer_texture_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void model_viewer_texture_resource::_on_loaded(
  const gl_texture_resource::load_info&) noexcept {
    signal_loaded();
}
//------------------------------------------------------------------------------
auto model_viewer_texture_resource::is_loaded() noexcept -> bool {
    return gl_texture_resource::is_loaded();
}
//------------------------------------------------------------------------------
void model_viewer_texture_resource::load_if_needed(
  execution_context& ctx,
  video_context& video) {
    video.with_gl([&, this](auto&, auto& GL) {
        gl_texture_resource::load_if_needed(
          ctx, GL.texture_2d_array, GL.texture0 + texture_unit(video));
    });
}
//------------------------------------------------------------------------------
void model_viewer_texture_resource::use(video_context&) {}
//------------------------------------------------------------------------------
auto model_viewer_texture_resource::texture_unit(video_context&)
  -> oglplus::texture_unit::value_type {
    // TODO
    return 0;
}
//------------------------------------------------------------------------------
void model_viewer_texture_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_texture_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_texture>,
  url locator,
  execution_context& ctx,
  video_context& video) -> model_viewer_texture_holder {
    return {
      hold<model_viewer_texture_resource>, std::move(locator), ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
