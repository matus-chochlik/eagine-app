/// @example app/037_lantern/resources.hpp
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
// programs
//------------------------------------------------------------------------------
class draw_program : public gl_program_resource {
public:
    draw_program(execution_context&);

    void set_camera(video_context&, const orbiting_camera& camera);
    void set_candle_light(video_context&, oglplus::gl_types::float_type);
    void set_ambient_light(video_context&, oglplus::gl_types::float_type);
    void set_texture_unit(video_context&, oglplus::texture_unit::value_type);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _candle_light_loc;
    oglplus::uniform_location _ambient_light_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
class screen_program : public gl_program_resource {
public:
    screen_program(execution_context&);

    void set_screen_size(video_context& vc);
    void set_texture_unit(video_context&, oglplus::texture_unit::value_type);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location _screen_size_loc;
    oglplus::uniform_location _tex_loc;
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class pumpkin_geometry : public gl_geometry_and_bindings_resource {
public:
    pumpkin_geometry(execution_context&);

    auto bounding_sphere() const noexcept {
        return _bounding_sphere;
    }

private:
    void _on_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;
    oglplus::sphere _bounding_sphere;
};
//------------------------------------------------------------------------------
class pumpkin_texture : public gl_texture_resource {

public:
    pumpkin_texture(execution_context&);

    auto load_if_needed(execution_context&) noexcept -> work_done;

    static auto tex_unit() noexcept -> oglplus::texture_unit::value_type {
        return 0U;
    }

private:
    void _on_loaded(const gl_texture_resource::load_info&) noexcept;
};
//------------------------------------------------------------------------------
class screen_geometry : public gl_geometry_and_bindings_resource {
public:
    screen_geometry(execution_context&);
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

    auto tex_unit() const noexcept -> oglplus::texture_unit::value_type {
        return 1U;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
