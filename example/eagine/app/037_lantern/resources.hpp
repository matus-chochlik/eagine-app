/// @example app/037_lantern/resources.hpp
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
// programs
//------------------------------------------------------------------------------
class draw_program : public gpu_program {
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
class screen_program : public gpu_program {
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
class pumpkin_model {

public:
    pumpkin_model(video_context&, resource_loader&);

    void update(video_context&, resource_loader&) noexcept;
    void clean_up(video_context&, resource_loader&) noexcept;

    explicit operator bool() const noexcept {
        return bool(_geom);
    }

    static auto tex_unit() noexcept -> oglplus::gl_types::int_type {
        return 0;
    }

    auto geometry() const noexcept -> const auto& {
        return _geom;
    }

    auto bounding_sphere() const noexcept {
        return _bounding_sphere;
    }

private:
    void _on_geom_loaded(
      const geometry_and_bindings_resource::load_info& info) noexcept {
        _bounding_sphere = info.shape.bounding_sphere();
    }

    geometry_and_bindings_resource _geom;
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
