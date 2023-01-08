/// @example app/028_compute_particles/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;

#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
example::example(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _bg{_video, {0.45F, 0.40F, 0.35F, 1.0F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _emit_prog{*this}
  , _draw_prog{*this}
  , _path{*this} {
    _emit_prog.base_loaded.connect(
      make_callable_ref<&example::_on_resource_loaded>(this));
    _draw_prog.base_loaded.connect(
      make_callable_ref<&example::_on_resource_loaded>(this));

    _particles.init(*this);

    _camera.set_near(0.1F)
      .set_far(200.F)
      .set_orbit_min(10.0F)
      .set_orbit_max(40.0F)
      .set_fov(right_angle_());

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.enable(GL.depth_test);
    gl.disable(GL.cull_face);
}
//------------------------------------------------------------------------------
void example::_on_resource_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_emit_prog)) {
        _emit_prog.bind_random(*this, _particles.random_binding());
        _emit_prog.bind_offsets(*this, _particles.offsets_binding());
        _emit_prog.bind_velocities(*this, _particles.velocities_binding());
        _emit_prog.bind_ages(*this, _particles.ages_binding());
    }
    if(loaded.is(_draw_prog)) {
        _draw_prog.bind_origin_location(*this, _particles.origin_loc());
        _draw_prog.bind_offsets(*this, _particles.offsets_binding());
        _draw_prog.bind_ages(*this, _particles.ages_binding());
    }
}
//------------------------------------------------------------------------------
void example::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 7.F);
    }

    _bg.clear(_video, _camera);

    if(_path and _emit_prog and _draw_prog) {
        _emit_prog.prepare_frame(*this);
        _particles.emit(*this);

        _draw_prog.prepare_frame(*this);
        _particles.draw(*this);
    } else {
        _path.load_if_needed(context());
        _emit_prog.load_if_needed(context());
        _draw_prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example::clean_up() noexcept {
    _bg.clean_up(_video);
    _particles.clean_up(*this);
    _draw_prog.clean_up(context());
    _emit_prog.clean_up(context());
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

        return gl.disable and gl.clear_color and gl.create_shader and
               gl.shader_source and gl.compile_shader and gl.create_program and
               gl.attach_shader and gl.link_program and gl.use_program and
               gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
               gl.gen_vertex_arrays and gl.bind_vertex_array and
               gl.get_attrib_location and gl.vertex_attrib_pointer and
               gl.enable_vertex_attrib_array and gl.draw_arrays and
               GL.vertex_shader and GL.tess_control_shader and
               GL.tess_evaluation_shader and GL.vertex_shader and
               GL.geometry_shader and GL.fragment_shader and GL.compute_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
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
}; // namespace eagine::application
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
