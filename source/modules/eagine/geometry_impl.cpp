/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.shapes;
import eagine.oglplus;
import :context;

namespace eagine::app {
//------------------------------------------------------------------------------
gl_geometry_and_bindings::gl_geometry_and_bindings(
  const shared_holder<shapes::generator>& gen,
  const oglplus::vertex_attrib_bindings& bindings,
  const shapes::drawing_variant var,
  video_context& vc,
  memory::buffer& temp) noexcept
  : gl_geometry_and_bindings{
      oglplus::shape_generator{vc.gl_api(), gen},
      bindings,
      var,
      vc,
      temp} {}
//------------------------------------------------------------------------------
gl_geometry_and_bindings::gl_geometry_and_bindings(
  const shared_holder<shapes::generator>& gen,
  const oglplus::vertex_attrib_bindings& bindings,
  video_context& vc,
  memory::buffer& temp) noexcept
  : gl_geometry_and_bindings{
      oglplus::shape_generator{vc.gl_api(), gen},
      bindings,
      vc,
      temp} {}
//------------------------------------------------------------------------------
gl_geometry_and_bindings::gl_geometry_and_bindings(
  const oglplus::shape_generator& shape,
  video_context& vc,
  memory::buffer& temp) noexcept
  : gl_geometry_and_bindings{
      shape,
      oglplus::vertex_attrib_bindings{
        shapes::shared_vertex_attrib_variants::basic(),
        shape},
      vc,
      temp} {}
//------------------------------------------------------------------------------
gl_geometry_and_bindings::gl_geometry_and_bindings(
  const shared_holder<shapes::generator>& gen,
  video_context& vc,
  memory::buffer& temp) noexcept
  : gl_geometry_and_bindings{
      oglplus::shape_generator{vc.gl_api(), gen},
      vc,
      temp} {}
//------------------------------------------------------------------------------
auto gl_geometry_and_bindings::init(gl_geometry_and_bindings&& temp) noexcept
  -> gl_geometry_and_bindings& {
    *this = std::move(temp);
    return *this;
}
//------------------------------------------------------------------------------
auto gl_geometry_and_bindings::reinit(
  video_context& vc,
  gl_geometry_and_bindings&& temp) noexcept -> gl_geometry_and_bindings& {
    clean_up(vc);
    *this = std::move(temp);
    return *this;
}
//------------------------------------------------------------------------------
auto gl_geometry_and_bindings::clean_up(video_context& vc) noexcept
  -> gl_geometry_and_bindings& {
    base::clean_up(vc.gl_api());
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
