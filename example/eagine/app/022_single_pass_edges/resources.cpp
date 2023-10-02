/// @example application/022_single_pass_edges/resources.cpp
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
edges_program::edges_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&edges_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void edges_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Projection") >> camera_loc;
    info.get_uniform_location("ViewportDimensions") >> vp_dim_loc;
}
//------------------------------------------------------------------------------
void edges_program::set_projection(
  execution_context& ec,
  orbiting_camera& camera) {
    auto& vc = ec.main_video();
    const auto [width, height] = vc.surface_size();
    set(vc, camera_loc, camera.matrix(vc));
    set(vc, vp_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void icosahedron_geometry::init(execution_context& ec) {
    gl_geometry_and_bindings::init(
      {shapes::center(eagine::shapes::ortho_array_xyz(
         shapes::unit_icosahedron(shapes::vertex_attrib_kind::position),
         {1.F, 1.F, 1.F},
         {3, 3, 3})),
       ec.main_video()});
}
//------------------------------------------------------------------------------
} // namespace eagine::app
