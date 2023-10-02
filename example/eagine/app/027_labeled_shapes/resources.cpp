/// @example application/027_labeled_shapes/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
draw_program::draw_program(execution_context& ctx)
  : gl_program_resource{url{"json:///Program"}, ctx} {
    loaded.connect(make_callable_ref<&draw_program::_on_loaded>(this));
}
//------------------------------------------------------------------------------
void draw_program::_on_loaded(
  const gl_program_resource::load_info& info) noexcept {
    info.get_uniform_location("Camera") >> camera_loc;
    info.get_uniform_location("Model") >> model_loc;
}
//------------------------------------------------------------------------------
void draw_program::set_projection(video_context& video, const mat4& projection) {
    set(video, camera_loc, projection);
}
//------------------------------------------------------------------------------
void draw_program::set_model(video_context& video, const mat4& model) {
    set(video, model_loc, model);
}
//------------------------------------------------------------------------------
// example shapes
//------------------------------------------------------------------------------
example_shapes::example_shapes(execution_context& ec)
  : base{
      {{ec,
        "icosahedron",
        url{"shape:///unit_icosahedron?position=true+normal=true"},
        2.F,
        0.F,
        0.F},
       {ec,
        "sphere",
        url{"shape:///unit_sphere?position=true+normal=true"},
        0.62F,
        0.F,
        1.9F},
       {ec,
        "torus",
        url{"shape:///unit_torus?position=true+normal=true"},
        -1.62F,
        0.F,
        1.18F},
       {ec,
        "twisted torus",
        url{"shape:///unit_twisted_torus?position=true+normal=true"},
        -1.62F,
        0.F,
        -1.18F},
       {ec,
        "round_cube",
        url{"shape:///unit_round_cube?position=true+normal=true"},
        0.62F,
        0.F,
        -1.9F}}} {}
//------------------------------------------------------------------------------
example_shapes::operator bool() const noexcept {
    for(auto& shape : *this) {
        if(not shape.geometry) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
void example_shapes::load_if_needed(execution_context& ec) noexcept {
    for(auto& shape : *this) {
        shape.geometry.load_if_needed(ec);
    }
}
//------------------------------------------------------------------------------
void example_shapes::clean_up(execution_context& ec) noexcept {
    for(auto& shape : *this) {
        shape.geometry.clean_up(ec);
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app
