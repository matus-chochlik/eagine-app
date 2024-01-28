/// @example app/016_torus/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_torus : public timeouting_application {
public:
    example_torus(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_torus::_on_resource_loaded>(this);
    }

    video_context& _video;

    orbiting_camera camera;
    torus_geometry torus;
    torus_program prog;
};
//------------------------------------------------------------------------------
example_torus::example_torus(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , torus{context()}
  , prog{context()} {
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
void example_torus::_on_resource_loaded(const loaded_resource_base&) noexcept {
    if(torus and prog) {
        prog.apply_input_bindings(_video, torus);
        reset_timeout();
    }
}
//------------------------------------------------------------------------------
void example_torus::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state);
    }

    if(torus and prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        prog.set_projection(_video, camera);
        torus.use_and_draw(_video);
    } else {
        torus.load_if_needed(context());
        prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_torus::clean_up() noexcept {
    prog.clean_up(context());
    torus.clean_up(context());

    _video.end();
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
               GL.vertex_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_torus>(ec);
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
