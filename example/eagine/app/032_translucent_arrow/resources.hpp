/// @example app/032_translucent_arrow/resources.hpp
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
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class depth_program : public gl_program_resource {
public:
    depth_program(execution_context&);
    void set_camera(video_context&, orbiting_camera& camera);
    void update(video_context&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
class draw_program : public gl_program_resource {
public:
    draw_program(execution_context&);
    void set_depth_texture(video_context&, oglplus::gl_types::int_type);
    void set_camera(video_context&, orbiting_camera& camera);
    void update(execution_context&, video_context&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location depth_tex_loc;

    radians_t<float> rad{0.F};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_geometry : public gl_geometry_and_bindings {
public:
    void init(const std::shared_ptr<shapes::generator>&, video_context&);

    auto bounding_sphere() noexcept {
        return bound_sphere;
    }

private:
    oglplus::sphere bound_sphere;
};
//------------------------------------------------------------------------------
class depth_texture {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void reshape(video_context&);
    void copy_from_fb(video_context&);

    auto texture_unit() const noexcept {
        return tex_unit;
    }

private:
    oglplus::gl_types::int_type tex_unit{0};
    oglplus::owned_texture_name tex{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
