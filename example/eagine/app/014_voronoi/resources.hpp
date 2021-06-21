/// @example app/014_voronoi/resources.hpp
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

namespace eagine::app {
//------------------------------------------------------------------------------
// random texture
//------------------------------------------------------------------------------
class random_texture {
public:
    void init(execution_context&, video_context&);
    void clean_up(execution_context&, video_context&);

private:
    oglplus::owned_texture_name random;
};
//------------------------------------------------------------------------------
// voronoi program
//------------------------------------------------------------------------------
class voronoi_program {
public:
    oglplus::owned_program_name prog;
    oglplus::uniform_location offset_loc;
    oglplus::uniform_location scale_loc;

    void init(execution_context&, video_context&);
    void clean_up(execution_context&, video_context&);

private:
    oglplus::owned_shader_name vs;
    oglplus::owned_shader_name fs;
};
//------------------------------------------------------------------------------
// screen geometry
//------------------------------------------------------------------------------
class screen_geometry {
public:
    oglplus::vertex_attrib_location position_loc{0};
    oglplus::vertex_attrib_location tex_coord_loc{1};

    void init(execution_context&, video_context&);
    void clean_up(execution_context&, video_context&);

private:
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name tex_coords;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
