/// @example app/024_overdraw/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include "main.hpp"
#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// example
//------------------------------------------------------------------------------
example::example(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _draw_prog{*this}
  , _screen_prog{*this} {
    _draw_prog.load_event.connect(
      make_callable_ref<&example::_on_load_event>(this));
    _screen_prog.load_event.connect(
      make_callable_ref<&example::_on_load_event>(this));

    _shape.init(*this);
    _screen.init(*this);
    _draw_bufs.init(*this);

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(11.0F)
      .set_orbit_max(16.0F)
      .set_fov(right_angle_());
    _draw_prog.set_projection(*this);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
    gl.blend_func(GL.one, GL.one);
    gl.clear_color(0.F, 0.F, 0.F, 0.F);
}
//------------------------------------------------------------------------------
void example::_on_load_event(const loaded_resource_base& loaded) noexcept {
    if(loaded.is_loaded(_draw_prog)) {
        _draw_prog.bind_position_location(*this, _shape.position_loc());
    }
    if(loaded.is_loaded(_screen_prog)) {
        _screen_prog.bind_position_location(*this, _screen.position_loc());
        _screen_prog.bind_tex_coord_location(*this, _screen.tex_coord_loc());
    }
}
//------------------------------------------------------------------------------
void example::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
    _draw_bufs.resize(*this);
}
//------------------------------------------------------------------------------
void example::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    if(_draw_prog and _screen_prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        // draw offscreen
        _draw_bufs.draw_offscreen(*this);

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        gl.enable(GL.depth_test);
        gl.enable(GL.blend);

        _draw_prog.use(context());
        _draw_prog.set_projection(*this);
        _shape.draw(*this);

        gl.disable(GL.blend);
        gl.disable(GL.depth_test);

        // draw onscreen
        _draw_bufs.draw_onscreen(*this);

        _screen_prog.use(context());
        _screen_prog.set_screen_size(*this);
        _screen.draw(*this);
    } else {
        _draw_prog.load_if_needed(context());
        _screen_prog.load_if_needed(context());
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example::clean_up() noexcept {
    _cleanup.clear();
    _video.end();
}
//------------------------------------------------------------------------------
// launchpad
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options&) -> bool final;
    auto check_requirements(video_context&) -> bool final;
    auto launch(execution_context&, const launch_options&)
      -> unique_holder<application> final;
};
//------------------------------------------------------------------------------
auto example_launchpad::setup(main_ctx&, launch_options& opts) -> bool {
    opts.no_audio().require_input().require_video();
    return true;
}
//------------------------------------------------------------------------------
auto example_launchpad::check_requirements(video_context& vc) -> bool {
    const auto& [gl, GL] = vc.gl_api();

    return gl.disable and gl.clear_color and gl.create_shader and
           gl.shader_source and gl.compile_shader and gl.create_program and
           gl.attach_shader and gl.link_program and gl.use_program and
           gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
           gl.gen_vertex_arrays and gl.bind_vertex_array and
           gl.get_attrib_location and gl.vertex_attrib_pointer and
           gl.enable_vertex_attrib_array and gl.draw_arrays and
           GL.vertex_shader and GL.fragment_shader;
}
//------------------------------------------------------------------------------
auto example_launchpad::launch(execution_context& ec, const launch_options&)
  -> unique_holder<application> {
    return launch_with_video<example>(ec);
}
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
