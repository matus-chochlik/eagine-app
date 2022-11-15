/// @example app/024_overdraw/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
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
    _draw_prog.base_loaded.connect(
      make_callable_ref<&example::_on_resource_loaded>(this));
    _screen_prog.base_loaded.connect(
      make_callable_ref<&example::_on_resource_loaded>(this));

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
void example::_on_resource_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_draw_prog)) {
        _draw_prog.bind_position_location(*this, _shape.position_loc());
    }
    if(loaded.is(_screen_prog)) {
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

    if(_draw_prog && _screen_prog) {
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

    _video.commit();
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
    auto check_requirements(video_context&) -> bool;
    auto launch(execution_context&, const launch_options&)
      -> std::unique_ptr<application> final;
};
//------------------------------------------------------------------------------
auto example_launchpad::setup(main_ctx&, launch_options& opts) -> bool {
    opts.no_audio().require_input().require_video();
    return true;
}
//------------------------------------------------------------------------------
auto example_launchpad::check_requirements(video_context& vc) -> bool {
    const auto& [gl, GL] = vc.gl_api();

    return gl.disable && gl.clear_color && gl.create_shader &&
           gl.shader_source && gl.compile_shader && gl.create_program &&
           gl.attach_shader && gl.link_program && gl.use_program &&
           gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
           gl.gen_vertex_arrays && gl.bind_vertex_array &&
           gl.get_attrib_location && gl.vertex_attrib_pointer &&
           gl.enable_vertex_attrib_array && gl.draw_arrays &&
           GL.vertex_shader && GL.fragment_shader;
}
//------------------------------------------------------------------------------
auto example_launchpad::launch(execution_context& ec, const launch_options&)
  -> std::unique_ptr<application> {
    if(auto opt_vc{ec.video_ctx()}) {
        auto& vc = extract(opt_vc);
        vc.begin();
        if(vc.init_gl_api()) {
            if(check_requirements(vc)) {
                return {std::make_unique<example>(ec, vc)};
            }
        }
    }
    return {};
}
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
