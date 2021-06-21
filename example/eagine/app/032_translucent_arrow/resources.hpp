/// @example app/032_translucent_arrow/resources.hpp
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
#include <eagine/cleanup_group.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/quantities.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class depth_program {
public:
    void init(execution_context&, video_context&, cleanup_group&);
    void set_camera(video_context& ctx, orbiting_camera& camera);
    void update(video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;

    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
class draw_program {
public:
    void init(execution_context&, video_context&, cleanup_group&);
    void set_depth_texture(video_context& ctx, oglplus::gl_types::int_type);
    void set_camera(video_context&, orbiting_camera& camera);
    void update(execution_context&, video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void
    bind_normal_location(video_context& ctx, oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;

    oglplus::uniform_location camera_loc;
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location depth_tex_loc;

    radians_t<float> rad{0.F};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class arrow_geometry {
public:
    void init(execution_context&, video_context&, cleanup_group&);
    void draw(video_context&);

    auto bounding_sphere() noexcept {
        return bound_sphere;
    }

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto normal_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

private:
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name indices;

    std::vector<oglplus::shape_draw_operation> ops;

    oglplus::sphere bound_sphere;
};
//------------------------------------------------------------------------------
class depth_texture {
public:
    void init(execution_context&, video_context&, cleanup_group&);
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
