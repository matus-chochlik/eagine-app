/// @example app/037_lantern/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include "resources.hpp"
#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/app_config.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/timeout.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_lantern : public application {
public:
    example_lantern(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{30}};

    pumpkin_geometry pumpkin;
    draw_program draw_prog;

    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_lantern::example_lantern(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    pumpkin.init(_video);
    draw_prog.init(_video);

    draw_prog.bind_position_location(_video, pumpkin.position_loc());
    draw_prog.bind_normal_location(_video, pumpkin.normal_loc());
    draw_prog.bind_wrap_coord_location(_video, pumpkin.wrap_coord_loc());
    draw_prog.set_texture_unit(_video, pumpkin.tex_unit());

    // camera
    const auto bs = pumpkin.bounding_sphere();
    const auto sr = bs.radius();
    camera.set_pitch_max(degrees_(89.F))
      .set_pitch_min(degrees_(-1.F))
      .set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 2.9F)
      .set_fov(degrees_(70));

    gl.clear_color(0.05F, 0.05F, 0.05F, 1.0F);
    gl.enable(GL.depth_test);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_lantern::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport(_video.surface_size());
}
//------------------------------------------------------------------------------
void example_lantern::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 5.F);
    }

    const auto t = state.frame_time().value();
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);

    draw_prog.set_camera(_video, camera);
    draw_prog.set_light_power(
      _video,
      std::sin(t * 0.618F * 11.F) * 0.1F + std::cos(t * 1.618F * 7.F) * 0.1F +
        0.9F);

    pumpkin.draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_lantern::clean_up() noexcept {

    draw_prog.clean_up(_video);
    pumpkin.clean_up(_video);

    _video.end();
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable && gl.clear_color && gl.create_shader &&
               gl.shader_source && gl.compile_shader && gl.create_program &&
               gl.attach_shader && gl.link_program && gl.use_program &&
               gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
               gl.gen_vertex_arrays && gl.bind_vertex_array &&
               gl.get_attrib_location && gl.vertex_attrib_pointer &&
               gl.enable_vertex_attrib_array && gl.draw_arrays &&
               gl.tex_image2d && GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_lantern>(ec, vc)};
                }
            }
        }
        return {};
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> std::unique_ptr<launchpad> {
    return {std::make_unique<example_launchpad>()};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
