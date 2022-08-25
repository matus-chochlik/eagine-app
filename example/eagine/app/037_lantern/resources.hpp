/// @example app/037_lantern/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

#if EAGINE_APP_MODULE
import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
#else
#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/framebuffer.hpp>
#include <eagine/app/fwd.hpp>
#include <eagine/app/geometry.hpp>
#include <eagine/app/gpu_program.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#endif

namespace eagine::app {
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class draw_program : public glsl_program {
public:
    void init(video_context&);
    void set_camera(video_context&, const orbiting_camera& camera);
    void set_candle_light(video_context&, oglplus::gl_types::float_type);
    void set_ambient_light(video_context&, oglplus::gl_types::float_type);
    void set_texture_unit(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_normal_location(video_context&, oglplus::vertex_attrib_location);
    void bind_wrap_coord_location(
      video_context&,
      oglplus::vertex_attrib_location);

private:
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _candle_light_loc;
    oglplus::uniform_location _ambient_light_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
class screen_program : public glsl_program {
public:
    void init(video_context&);
    void set_screen_size(video_context& vc);
    void set_texture_unit(video_context&, oglplus::gl_types::int_type);

    void bind_position_location(video_context&, oglplus::vertex_attrib_location);
    void bind_coord_location(video_context&, oglplus::vertex_attrib_location);

private:
    oglplus::uniform_location _screen_size_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class pumpkin_geometry : public geometry_and_bindings {
public:
    void init(video_context&);
    void clean_up(video_context&);

    auto tex_unit() const noexcept -> oglplus::gl_types::int_type {
        return 0;
    }

    auto bounding_sphere() const noexcept {
        return _bounding_sphere;
    }

private:
    oglplus::sphere _bounding_sphere;
    oglplus::owned_texture_name _tex{};
};
//------------------------------------------------------------------------------
class screen_geometry : public geometry_and_bindings {
public:
    void init(video_context&);
};
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
class draw_buffers : public offscreen_framebuffer {
    using base = offscreen_framebuffer;

public:
    void init(video_context&);
    void resize(video_context&);

    void draw_off_screen(video_context&);
    void draw_on_screen(video_context&);

    auto tex_unit() const noexcept -> oglplus::gl_types::int_type {
        return 1;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
