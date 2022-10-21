/// @example app/016_torus/resources.cpp
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
torus_program::torus_program(video_context& video, resource_loader& loader)
  : gl_program_resource{url{"json:///GLProgram"}, video, loader} {
    loaded.connect(make_callable_ref<&torus_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void torus_program::set_projection(video_context& vc, orbiting_camera& camera) {
    if(camera.has_changed()) {
        vc.gl_api().set_uniform(
          *this, camera_loc, camera.matrix(vc.surface_aspect()));
    }
}
//------------------------------------------------------------------------------
void torus_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.use_program();
    info.get_uniform_location("Camera") >> camera_loc;
    input_bindings = info.base.input_bindings;
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
torus_geometry::torus_geometry(video_context& video, resource_loader& loader)
  : gl_geometry_and_bindings_resource{
      url{"shape:///unit_torus?position=true+normal=true+wrap_coord=true"},
      video,
      loader} {}
//------------------------------------------------------------------------------
} // namespace eagine::app
