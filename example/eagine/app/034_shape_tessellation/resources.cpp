/// @example app/034_shape_tessellation/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
tess_program::tess_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&tess_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void tess_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("CameraMatrix") >> camera_matrix_loc;
    info.get_uniform_location("CameraPosition") >> camera_position_loc;
    info.get_uniform_location("ViewportDimensions") >> viewport_dim_loc;
    info.get_uniform_location("Factor") >> factor_loc;
}
//------------------------------------------------------------------------------
void tess_program::set_factor(video_context& vc, float factor) {
    set(vc, factor_loc, factor);
}
//------------------------------------------------------------------------------
void tess_program::set_projection(video_context& vc, orbiting_camera& camera) {
    const auto [width, height] = vc.surface_size();
    set(vc, camera_position_loc, camera.position());
    set(vc, camera_matrix_loc, camera.matrix(vc));
    set(vc, viewport_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
tess_geometry::tess_geometry(url locator, execution_context& ctx)
  : gl_geometry_and_bindings_resource{std::move(locator), ctx} {
    loaded.connect(make_callable_ref<&tess_geometry::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void tess_geometry::_on_loaded(
  const gl_geometry_and_bindings_resource::load_info& info) noexcept {
    _bounding_sphere = info.base.shape.bounding_sphere();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
