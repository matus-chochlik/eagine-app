/// @example app/034_shape_tessellation/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
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
class tess_program : public gl_program_resource {
public:
    tess_program(execution_context&);
    void set_projection(video_context&, orbiting_camera&);
    void set_factor(video_context&, float);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location camera_matrix_loc;
    oglplus::uniform_location camera_position_loc;
    oglplus::uniform_location viewport_dim_loc;
    oglplus::uniform_location factor_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class tess_geometry : public gl_geometry_and_bindings_resource {
public:
    tess_geometry(url, execution_context&);

    auto bounding_sphere() const noexcept {
        return _bounding_sphere;
    }

private:
    void _on_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;
    oglplus::sphere _bounding_sphere;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
