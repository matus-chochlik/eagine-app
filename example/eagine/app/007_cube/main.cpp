/// @example app/007_cube/main.cpp
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

namespace eagine::app {
//------------------------------------------------------------------------------
class example_cube : public application {
public:
    example_cube(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_prog_loaded(
      oglplus::program_name prog,
      const oglplus::program_input_bindings& input_bindings) noexcept {
        auto& glapi = _video.gl_api();
        input_bindings.apply(glapi, prog, _cube);
        glapi.use_program(prog);
        glapi.get_uniform_location(prog, "Camera") >> _camera_loc;
    }

    execution_context& _ctx;
    video_context& _video;
    resource_loader& _loader;
    background_color_depth _bg;
    timeout _is_done{std::chrono::seconds{30}};

    oglplus::vertex_attrib_bindings _attrib_bindings;
    loaded_resource<geometry_and_bindings> _cube;
    loaded_resource<oglplus::owned_program_name> _prog;

    orbiting_camera _camera;
    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _loader{_ctx.loader()}
  , _bg{0.4F, 0.F, 1.F}
  , _cube{url{"shape:///unit_cube?position=true+normal=true"}, _video, _loader}
  , _prog{url{"json:///GLProgram"}, _video, _loader} {
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
void example_cube::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_cube::update() noexcept {
    if(!_cube) {
        _cube.update(_video, _loader);
    } else {
        if(!_prog) {
            _prog.update(_video, _loader);
        }
    }

    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    const auto& glapi = _video.gl_api();

    _bg.clear(_video);

    if(_cube && _prog) {
        glapi.set_uniform(_prog, _camera_loc, _camera.matrix(_video));
        _cube.draw(glapi);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {
    _prog.clean_up(_video, _loader);
    _cube.clean_up(_video, _loader);

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
                    return {std::make_unique<example_cube>(ec, vc)};
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
