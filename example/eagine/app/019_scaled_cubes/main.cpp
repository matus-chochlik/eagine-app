/// @example app/019_scaled_cubes/main.cpp
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
class example_cubes : public timeouting_application {
public:
    example_cubes(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    video_context& _video;

    cubes_program _prog;
    cubes_geometry _cubes;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_cubes::example_cubes(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _prog{ec} {
    _prog.loaded.connect(make_callable_ref<&example_cubes::_on_loaded>(this));

    _cubes.init(ec, vc);

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(10.2F)
      .set_orbit_max(16.0F)
      .set_fov(degrees_(70));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    gl.clear_color(0.45F, 0.45F, 0.45F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_cubes::_on_loaded(
  const gl_program_resource::load_info& loaded) noexcept {
    loaded.apply_input_bindings(_cubes.attrib_bindings());
    _prog.set_projection(_video, _camera);
}
//------------------------------------------------------------------------------
void example_cubes::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 3.F);
    }

    if(_prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _prog.update(context(), _video);
        _prog.set_projection(_video, _camera);

        _prog.drawing_surface(_video);
        _cubes.draw_surface(_video);

        _prog.drawing_edges(_video);
        _cubes.draw_edges(_video);
    } else {
        _prog.load_if_needed(context());
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_cubes::clean_up() noexcept {

    _prog.clean_up(context());
    _cubes.clean_up(context());

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
        return launch_with_video<example_cubes>(ec);
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
