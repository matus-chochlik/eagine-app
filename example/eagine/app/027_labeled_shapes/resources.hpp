/// @example app/027_labeled_shapes/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define EAGINE_APP_EXAMPLE_RESOURCES_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class draw_program : public gl_program_resource {
public:
    draw_program(execution_context&);
    void set_projection(video_context&, const mat4& projection);
    void set_model(video_context&, const mat4& model);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location model_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
struct example_shape_data {
    string_view label;
    gl_geometry_and_bindings_resource geometry;
    math::translation<mat4::base> transform;
    vec4 label_pos;
    vec2 screen_pos;

    example_shape_data(
      execution_context& ec,
      string_view lbl,
      url shape_url,
      float x,
      float y,
      float z) noexcept
      : label{lbl}
      , geometry{shape_url, ec}
      , transform{x, y, z}
      , label_pos{x, y + 0.5F, z, 1.F} {}
};
//------------------------------------------------------------------------------
struct example_shapes : std::array<example_shape_data, 5> {
    using base = std::array<example_shape_data, 5>;
    example_shapes(execution_context&);

    explicit operator bool() const noexcept;

    void load_if_needed(execution_context& ec) noexcept;
    void clean_up(execution_context& ec) noexcept;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
