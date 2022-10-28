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

    if(_path && _emit_prog && _draw_prog) {
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

        return gl.disable && gl.clear_color && gl.create_shader &&
               gl.shader_source && gl.compile_shader && gl.create_program &&
               gl.attach_shader && gl.link_program && gl.use_program &&
               gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
               gl.gen_vertex_arrays && gl.bind_vertex_array &&
               gl.get_attrib_location && gl.vertex_attrib_pointer &&
               gl.enable_vertex_attrib_array && gl.draw_arrays &&
               GL.vertex_shader && GL.tess_control_shader &&
               GL.tess_evaluation_shader && GL.vertex_shader &&
               GL.geometry_shader && GL.fragment_shader && GL.compute_shader;
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
