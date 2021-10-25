/// @example app/037_lantern/resources.hpp
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
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/oglplus/shapes/generator.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class draw_program {
public:
    void init(video_context&);
    void set_camera(video_context&, const orbiting_camera& camera);
    void set_light_power(video_context&, oglplus::gl_types::float_type);
    void set_texture_unit(video_context&, oglplus::gl_types::int_type);
    void use(video_context&);
    void clean_up(video_context&);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_wrap_coord_location(
      video_context&,
      oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location light_power_loc;
    oglplus::uniform_location tex_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class pumpkin_geometry {
public:
    void init(video_context&);
    void use(video_context&);
    void clean_up(video_context&);
    void draw(video_context&);

    auto position_loc() const noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    auto normal_loc() const noexcept {
        return oglplus::vertex_attrib_location{1};
    }

    auto wrap_coord_loc() const noexcept {
        return oglplus::vertex_attrib_location{2};
    }

    auto tex_unit() const noexcept -> oglplus::gl_types::int_type {
        return 0;
    }

    auto bounding_sphere() const noexcept {
        return bound_sphere;
    }

private:
    oglplus::sphere bound_sphere;
    std::vector<oglplus::shape_draw_operation> ops;
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name wrap_coords;
    oglplus::owned_buffer_name indices;

    oglplus::owned_texture_name tex{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
