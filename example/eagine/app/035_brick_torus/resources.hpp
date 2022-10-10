/// @example app/035_brick_torus/resources.hpp
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
class torus_program : public gpu_program {
public:
    void init(video_context&);
    void set_camera(video_context&, orbiting_camera& camera);
    void set_model(video_context&, const oglplus::trfmat<4>&);
    void set_light(video_context&, const oglplus::vec3&);
    void set_texture_map(video_context&, oglplus::gl_types::int_type unit);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_tangent_location(video_context&, oglplus::vertex_attrib_location);
    void bind_texcoord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location camera_pos_loc;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location model_loc;
    oglplus::uniform_location texture_map_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry : public geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
class torus_textures {
public:
    void init(video_context&);
    void clean_up(video_context&);

    static auto bricks_map_unit() noexcept {
        return oglplus::gl_types::int_type(0);
    }

    static auto stones_map_unit() noexcept {
        return oglplus::gl_types::int_type(1);
    }

private:
    oglplus::owned_texture_name bricks;
    oglplus::owned_texture_name stones;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
