/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.model_viewer;

import eagine.core;
import eagine.guiplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_geometry::draw(video_context& video)
  -> model_viewer_geometry& {
    assert(_impl);
    _impl->draw(video);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_geometry::bounding_sphere() noexcept -> oglplus::sphere {
    assert(_impl);
    return _impl->bounding_sphere();
}
//------------------------------------------------------------------------------
auto model_viewer_geometry::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    assert(_impl);
    return _impl->attrib_bindings();
}
//------------------------------------------------------------------------------
//  Geometry
//------------------------------------------------------------------------------
class model_viewer_geometry_resource
  : public model_viewer_geometry_intf
  , public gl_geometry_and_bindings_resource {
public:
    model_viewer_geometry_resource(
      url locator,
      execution_context&,
      video_context&);

    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    void draw(video_context&) final;

    auto bounding_sphere() noexcept -> oglplus::sphere final;
    auto attrib_bindings() noexcept
      -> const oglplus::vertex_attrib_bindings& final;
    void clean_up(execution_context&, video_context&) final;

private:
    void _on_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;
    oglplus::sphere _bounding_sphere;
};
//------------------------------------------------------------------------------
model_viewer_geometry_resource::model_viewer_geometry_resource(
  url locator,
  execution_context& ctx,
  video_context&)
  : gl_geometry_and_bindings_resource{std::move(locator), ctx} {
    gl_geometry_and_bindings_resource::loaded.connect(
      make_callable_ref<&model_viewer_geometry_resource::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void model_viewer_geometry_resource::_on_loaded(
  const gl_geometry_and_bindings_resource::load_info& info) noexcept {
    _bounding_sphere = info.base.shape.bounding_sphere();
    signal_loaded();
}
//------------------------------------------------------------------------------
auto model_viewer_geometry_resource::is_loaded() noexcept -> bool {
    return gl_geometry_and_bindings_resource::is_loaded();
}
//------------------------------------------------------------------------------
void model_viewer_geometry_resource::load_if_needed(
  execution_context& ctx,
  video_context&) {
    gl_geometry_and_bindings_resource::load_if_needed(ctx);
}
//------------------------------------------------------------------------------
auto model_viewer_geometry_resource::bounding_sphere() noexcept
  -> oglplus::sphere {
    return _bounding_sphere;
}
//------------------------------------------------------------------------------
auto model_viewer_geometry_resource::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    return *this;
}
//------------------------------------------------------------------------------
void model_viewer_geometry_resource::use(video_context& video) {
    gl_geometry_and_bindings_resource::use(video);
}
//------------------------------------------------------------------------------
void model_viewer_geometry_resource::draw(video_context& video) {
    gl_geometry_and_bindings_resource::draw(video);
}
//------------------------------------------------------------------------------
void model_viewer_geometry_resource::clean_up(
  execution_context& ctx,
  video_context& video) {
    gl_geometry_and_bindings_resource::clean_up(ctx.loader(), video);
}
//------------------------------------------------------------------------------
//  Default geometry
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_geometry>,
  url locator,
  execution_context& ctx,
  video_context& video) -> model_viewer_geometry_holder {
    return {
      hold<model_viewer_geometry_resource>, std::move(locator), ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
