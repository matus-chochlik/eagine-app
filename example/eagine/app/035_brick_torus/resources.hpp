/// @example app/035_brick_torus/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/fwd.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class torus_program {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void set_camera(video_context&, orbiting_camera& camera);
    void set_model(video_context&, const oglplus::tmat<float, 4, 4, true>&);
    void set_light(video_context&, const oglplus::vec3&);
    void set_bricks_map(video_context&, oglplus::gl_types::int_type unit);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_tangent_location(video_context&, oglplus::vertex_attrib_location);
    void bind_texcoord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location camera_pos_loc;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location model_loc;
    oglplus::uniform_location bricks_map_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class torus_geometry {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);
    void draw(execution_context&, video_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto normal_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

    static auto tangent_loc() noexcept {
        return oglplus::vertex_attrib_location{2};
    }

    static auto texcoord_loc() noexcept {
        return oglplus::vertex_attrib_location{3};
    }

private:
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name tangents;
    oglplus::owned_buffer_name texcoords;
    oglplus::owned_buffer_name indices;

    std::vector<oglplus::shape_draw_operation> ops;
};
//------------------------------------------------------------------------------
// textures
//------------------------------------------------------------------------------
class torus_textures {
public:
    void init(execution_context&, video_context&);
    void clean_up(video_context&);

    static auto bricks_map_unit() noexcept {
        return oglplus::gl_types::int_type(0);
    }

private:
    oglplus::owned_texture_name color_hmap_nmap;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif