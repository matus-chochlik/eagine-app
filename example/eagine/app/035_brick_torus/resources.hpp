/// @example app/035_brick_torus/resources.hpp
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
class torus_program : public gl_program_resource {
public:
    torus_program(execution_context&);
    void set_camera(video_context&, orbiting_camera& camera);
    void set_model(video_context&, const oglplus::trfmat<4>&);
    void set_light(video_context&, const oglplus::vec3&);
    void set_texture_map(video_context&, oglplus::gl_types::int_type unit);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location camera_pos_loc;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location model_loc;
    oglplus::uniform_location texture_map_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry : public gl_geometry_and_bindings_resource {
public:
    torus_geometry(execution_context&);
};
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
class example_texture : public gl_texture_resource {
public:
    example_texture(url, oglplus::gl_types::int_type, execution_context&);

    auto load_if_needed(execution_context&) noexcept -> work_done;

    auto tex_unit() const noexcept {
        return _tex_unit;
    }

private:
    void _on_loaded(const gl_texture_resource::load_info&) noexcept;
    oglplus::gl_types::int_type _tex_unit;
};
//------------------------------------------------------------------------------
class brick_texture : public example_texture {
public:
    brick_texture(execution_context&);
};
//------------------------------------------------------------------------------
class stone_texture : public example_texture {
public:
    stone_texture(execution_context&);
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
