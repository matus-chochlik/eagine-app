/// @example app/019_scaled_cubes/resources.hpp
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
// program
//------------------------------------------------------------------------------
class cubes_program : public gl_program_resource {
public:
    cubes_program(execution_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void update(execution_context&, video_context&);

    void drawing_surface(video_context&);
    void drawing_edges(video_context&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location center_loc;
    oglplus::uniform_location time_loc;
    oglplus::uniform_location edges_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class cubes_geometry {
public:
    void init(execution_context&, video_context&);
    void clean_up(execution_context&);
    void draw_surface(video_context&);
    void draw_edges(video_context&);

    static auto position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto pivot_loc() noexcept {
        return oglplus::vertex_attrib_location{1};
    }

    static auto coord_loc() noexcept {
        return oglplus::vertex_attrib_location{2};
    }

    static auto attrib_bindings() noexcept {
        return oglplus::vertex_attrib_bindings{
          shapes::vertex_attrib_kind::position,
          shapes::vertex_attrib_kind::pivot,
          shapes::vertex_attrib_kind::box_coord};
    }

private:
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name pivots;
    oglplus::owned_buffer_name coords;
    oglplus::owned_buffer_name indices;

    std::vector<oglplus::shape_draw_operation> ops{};
    std::array<oglplus::shape_draw_subset, 2> subs{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
