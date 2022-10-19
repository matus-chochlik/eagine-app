/// @example app/037_lantern/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import <cmath>;

#include "resources.hpp"

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
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_lantern::_on_resource_loaded>(this);
    }

    execution_context& _ctx;
    video_context& _video;
    resource_loader& _loader;
    timeout _is_done{std::chrono::seconds{90}};

    draw_buffers draw_bufs;
    pumpkin_texture pumpkin_tex;
    pumpkin_geometry pumpkin;
    screen_geometry screen;
    draw_program draw_prog;
    screen_program screen_prog;

    orbiting_camera camera;
    bool _has_all_resources{false};
};
//------------------------------------------------------------------------------
example_lantern::example_lantern(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _loader{_ctx.loader()}
  , pumpkin_tex{_video, _loader}
  , pumpkin{_video, _loader}
  , screen{_video, _loader}
  , draw_prog{_video, _loader}
  , screen_prog{_video, _loader} {

    pumpkin_tex.base_loaded.connect(_load_handler());
    pumpkin.base_loaded.connect(_load_handler());
    screen.base_loaded.connect(_load_handler());
    draw_prog.base_loaded.connect(_load_handler());
    screen_prog.base_loaded.connect(_load_handler());

    draw_bufs.init(_video);

    // camera
    camera.set_pitch_max(degrees_(30.F))
      .set_pitch_min(degrees_(-25.F))
      .set_fov(degrees_(50))
      .set_azimuth(degrees_(180));

    _video.gl_api().operations().clear_depth(1.0);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_lantern::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
    draw_bufs.resize(_video);
}
//------------------------------------------------------------------------------
void example_lantern::_on_resource_loaded(
  const loaded_resource_base& loaded) noexcept {

    if(loaded.is(pumpkin)) {
        const auto bs = pumpkin.bounding_sphere();
        const auto sr = bs.radius();
        camera.set_target(bs.center())
          .set_near(sr * 0.01F)
          .set_far(sr * 50.0F)
          .set_orbit_min(sr * 1.2F)
          .set_orbit_max(sr * 2.4F);
    }
    if(loaded.is_one_of(pumpkin, draw_prog)) {
        if(pumpkin && draw_prog) {
            const auto& glapi = _video.gl_api();
            draw_prog.use(_video);
            draw_prog.input_bindings.apply(glapi, draw_prog, pumpkin);
        }
    }
    if(loaded.is_one_of(pumpkin_tex, draw_prog)) {
        if(pumpkin_tex && draw_prog) {
            draw_prog.use(_video);
            draw_prog.set_texture_unit(_video, pumpkin_tex.tex_unit());
        }
    }
    if(loaded.is_one_of(screen, screen_prog)) {
        if(screen && screen_prog) {
            const auto& glapi = _video.gl_api();
            screen_prog.use(_video);
            screen_prog.input_bindings.apply(glapi, screen_prog, screen);
            screen_prog.set_texture_unit(_video, draw_bufs.tex_unit());
        }
    }
    _is_done.reset();
    _has_all_resources =
      pumpkin_tex && pumpkin && screen && draw_prog && screen_prog;
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

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    if(_has_all_resources) {
        const auto t = state.frame_time().value();
        const auto ambient =
          math::blend(0.05F, 0.30F, math::cosine_wave01(t / 5.F));
        const auto candle = std::sin(t * 0.618F * 11.F) * 0.2F +
                            std::cos(t * 1.618F * 7.F) * 0.2F + 0.9F;

        draw_bufs.draw_off_screen(_video);
        gl.enable(GL.depth_test);
        gl.clear_color(ambient, ambient, ambient, 0.F);
        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);

        draw_prog.use(_video);
        draw_prog.set_camera(_video, camera);
        draw_prog.set_ambient_light(_video, ambient);
        draw_prog.set_candle_light(_video, candle);

        pumpkin.use(_video);
        pumpkin.draw(_video);
        //
        draw_bufs.draw_on_screen(_video);
        gl.disable(GL.depth_test);

        screen_prog.use(_video);
        screen_prog.set_screen_size(_video);

        screen.use(_video);
        screen.draw(_video);
    } else {
        gl.clear_color(0.F, 0.F, 0.F, 0.F);
        gl.clear(GL.color_buffer_bit);

        pumpkin_tex.update(_video, _loader);
        if(!pumpkin) {
            pumpkin.update(_video, _loader);
        } else {
            draw_prog.update(_video, _loader);
        }
        if(!screen) {
            screen.update(_video, _loader);
        } else {
            screen_prog.update(_video, _loader);
        }
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_lantern::clean_up() noexcept {
    draw_prog.clean_up(_video, _loader);
    screen_prog.clean_up(_video, _loader);
    screen.clean_up(_video, _loader);
    pumpkin.clean_up(_video, _loader);
    pumpkin_tex.clean_up(_video, _loader);
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
               gl.tex_image2d && gl.framebuffer_texture2d &&
               gl.framebuffer_renderbuffer && GL.vertex_shader &&
               GL.fragment_shader;
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
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
