/// @example app/007_cube/main.cpp
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

namespace eagine::app {
//------------------------------------------------------------------------------
class example_cube : public timeouting_application {
public:
    example_cube(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_prog_loaded(const gl_program_resource::load_info& info) noexcept {
        info.apply_input_bindings(_cube);
        info.get_uniform_location("Camera") >> _camera_loc;
        info.use_program();
        reset_timeout();
    }

    video_context& _video;
    background_color_depth _bg;

    oglplus::vertex_attrib_bindings _attrib_bindings;
    loaded_resource<gl_geometry_and_bindings> _cube;
    loaded_resource<oglplus::owned_program_name> _prog;

    orbiting_camera _camera;
    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _bg{0.4F, 0.F, 1.F}
  , _cube{url{"shape:///unit_cube?position=true&normal=true"}, ec}
  , _prog{url{"json:///GLProgram"}, ec} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    _prog.loaded.connect(
      make_callable_ref<&example_cube::_on_prog_loaded>(this));

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.1F)
      .set_orbit_max(3.5F)
      .set_fov(right_angle_());

    gl.enable(GL.depth_test);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_cube::update() noexcept {
    if(not _cube) {
        _cube.load_if_needed(context());
    } else {
        if(not _prog) {
            _prog.load_if_needed(context());
        }
    }

    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    const auto& glapi = _video.gl_api();

    _bg.clear(_video);

    if(_cube and _prog) {
        glapi.set_uniform(_prog, _camera_loc, _camera.matrix(_video));
        _cube.draw(glapi);
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {
    _prog.clean_up(context());
    _cube.clean_up(context());

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
        return launch_with_video<example_cube>(ec);
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
