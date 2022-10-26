/// @example app/016_torus/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_torus : public application {
public:
    example_torus(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_torus::_on_resource_loaded>(this);
    }

    execution_context& _ctx;
    video_context& _video;
    resource_loader& _loader;

    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;
    torus_geometry torus;
    torus_program prog;
};
//------------------------------------------------------------------------------
example_torus::example_torus(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _loader{_ctx.loader()}
  , torus{_video, _loader}
  , prog{_video, _loader} {
    torus.base_loaded.connect(_load_handler());
    prog.base_loaded.connect(_load_handler());

    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(0.6F)
      .set_orbit_max(1.7F)
      .set_fov(right_angle_());
    prog.set_projection(vc, camera);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_torus::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_torus::_on_resource_loaded(const loaded_resource_base&) noexcept {
    if(torus && prog) {
        prog.apply_input_bindings(_video, torus);
        _is_done.reset();
    }
}
//------------------------------------------------------------------------------
void example_torus::update() noexcept {

    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state);
    }

    if(torus && prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        prog.set_projection(_video, camera);
        torus.use_and_draw(_video);
    } else {
        torus.update(_video, _loader);
        prog.update(_video, _loader);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_torus::clean_up() noexcept {
    prog.clean_up(_video, _loader);
    torus.clean_up(_video, _loader);

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
               GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_torus>(ec, vc)};
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
