/// @example app/037_lantern/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_lantern : public timeouting_application {
public:
    example_lantern(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_load_event(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_lantern::_on_load_event>(this);
    }

    puzzle_progress<5> _load_progress;
    video_context& _video;

    draw_buffers draw_bufs;
    pumpkin_texture pumpkin_tex;
    pumpkin_geometry pumpkin;
    screen_geometry screen;
    draw_program draw_prog;
    screen_program screen_prog;

    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_lantern::example_lantern(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{90}}
  , _load_progress{ec.progress(), "Loading resources"}
  , _video{vc}
  , pumpkin_tex{ec}
  , pumpkin{ec}
  , screen{ec}
  , draw_prog{ec}
  , screen_prog{ec} {

    pumpkin_tex.load_event.connect(_load_handler());
    pumpkin.load_event.connect(_load_handler());
    screen.load_event.connect(_load_handler());
    draw_prog.load_event.connect(_load_handler());
    screen_prog.load_event.connect(_load_handler());

    draw_bufs.init(_video);

    // camera
    camera.set_pitch_max(degrees_(30.F))
      .set_pitch_min(degrees_(-25.F))
      .set_fov(degrees_(60))
      .set_azimuth(degrees_(180));

    _video.gl_api().operations().clear_depth(1.0);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_lantern::_on_load_event(
  const loaded_resource_base& loaded) noexcept {

    if(loaded.is_loaded(pumpkin)) {
        const auto bs = pumpkin.bounding_sphere();
        const auto sr = bs.radius();
        camera.set_target(bs.center())
          .set_near(sr * 0.01F)
          .set_far(sr * 50.0F)
          .set_orbit_min(sr * 2.0F)
          .set_orbit_max(sr * 3.0F);
    }
    if(loaded.is_one_of(pumpkin, draw_prog)) {
        if(pumpkin and draw_prog) {
            draw_prog.use(_video);
            draw_prog.apply_input_bindings(_video, pumpkin);
        }
    }
    if(loaded.is_one_of(pumpkin_tex, draw_prog)) {
        if(pumpkin_tex and draw_prog) {
            draw_prog.use(_video);
            draw_prog.set_texture_unit(_video, pumpkin_tex.tex_unit());
        }
    }
    if(loaded.is_one_of(screen, screen_prog)) {
        if(screen and screen_prog) {
            screen_prog.use(_video);
            screen_prog.apply_input_bindings(_video, screen);
            screen_prog.set_texture_unit(_video, draw_bufs.tex_unit());
        }
    }
    reset_timeout();
    _load_progress.update_progress(
      pumpkin_tex, pumpkin, screen, draw_prog, screen_prog);
}
//------------------------------------------------------------------------------
void example_lantern::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 5.F);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    if(_load_progress.done()) {
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

        pumpkin_tex.load_if_needed(context());
        if(not pumpkin) {
            pumpkin.load_if_needed(context());
        } else {
            draw_prog.load_if_needed(context());
        }
        if(not screen) {
            screen.load_if_needed(context());
        } else {
            screen_prog.load_if_needed(context());
        }
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_lantern::clean_up() noexcept {
    draw_prog.clean_up(context());
    screen_prog.clean_up(context());
    screen.clean_up(context());
    pumpkin.clean_up(context());
    pumpkin_tex.clean_up(context());
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool final {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable and gl.clear_color and gl.create_shader and
               gl.shader_source and gl.compile_shader and gl.create_program and
               gl.attach_shader and gl.link_program and gl.use_program and
               gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
               gl.gen_vertex_arrays and gl.bind_vertex_array and
               gl.get_attrib_location and gl.vertex_attrib_pointer and
               gl.enable_vertex_attrib_array and gl.draw_arrays and
               gl.tex_image2d and gl.framebuffer_texture2d and
               gl.framebuffer_renderbuffer and GL.vertex_shader and
               GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_lantern>(ec);
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad> {
    return {hold<example_launchpad>};
}
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
