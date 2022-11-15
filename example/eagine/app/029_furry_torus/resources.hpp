/// @example app/029_furry_torus/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class surface_program : public gl_program_resource {
public:
    surface_program(execution_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void set_model(video_context&, const oglplus::trfmat<4>&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location model_loc;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
class hair_program : public gl_program_resource {
public:
    hair_program(execution_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void set_model(
      video_context&,
      const oglplus::trfmat<4>& prev,
      const oglplus::trfmat<4>& curr);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location prev_model_loc;
    oglplus::uniform_location curr_model_loc;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_surface : public gl_geometry_and_bindings {
public:
    void init(video_context&, const std::shared_ptr<shapes::generator>&);
};
//------------------------------------------------------------------------------
class shape_hair : public gl_geometry_and_bindings {
public:
    void init(video_context&, const std::shared_ptr<shapes::generator>&);
};
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
class shape_texture : public gl_texture_resource {
public:
    shape_texture(url, execution_context&);

private:
    void _on_loaded(const gl_texture_resource::load_info&) noexcept;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
