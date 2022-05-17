/// @example app/029_furry_torus/resources.hpp
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
#include <eagine/app/geometry.hpp>
#include <eagine/app/gpu_program.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class surface_program : public glsl_program {
public:
    void init(video_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void set_model(video_context&, const oglplus::trfmat<4>&);
    void set_texture(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_texcoord_location(video_context&, oglplus::vertex_attrib_location);
    void bind_occlusion_location(
      video_context&,
      oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location model_loc;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location texture_loc;
};
//------------------------------------------------------------------------------
class hair_program : public glsl_program {
public:
    void init(video_context&);
    void set_projection(video_context&, orbiting_camera& camera);
    void set_model(
      video_context&,
      const oglplus::trfmat<4>& prev,
      const oglplus::trfmat<4>& curr);
    void set_texture(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_texcoord_location(video_context&, oglplus::vertex_attrib_location);
    void bind_occlusion_location(
      video_context&,
      oglplus::vertex_attrib_location);

private:
    oglplus::owned_program_name prog;
    oglplus::uniform_location prev_model_loc;
    oglplus::uniform_location curr_model_loc;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location texture_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class shape_surface : public geometry_and_bindings {
public:
    void init(video_context&, const std::shared_ptr<shapes::generator>&);
};
//------------------------------------------------------------------------------
class shape_hair : public geometry_and_bindings {
public:
    void init(video_context&, const std::shared_ptr<shapes::generator>&);
};
//------------------------------------------------------------------------------
// texture
//------------------------------------------------------------------------------
class shape_textures {
public:
    void init(video_context&);
    void clean_up(video_context&);

    static auto map_unit_zebra() noexcept {
        return oglplus::gl_types::int_type(0);
    }

    static auto map_unit_monkey() noexcept {
        return oglplus::gl_types::int_type(1);
    }

private:
    oglplus::owned_texture_name zebra;
    oglplus::owned_texture_name monkey;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
