/// @example app/000_temp/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;
import eagine.app;

import <iostream>;
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
    friend class app::resource_loader;

    void handle_gl_program_loaded(
      identifier_t,
      oglplus::program_name,
      std::reference_wrapper<oglplus::owned_program_name> ref,
      const oglplus::program_input_bindings& input_bindings,
      const url&) noexcept {
        auto& glapi = _video.gl_api();
        _prog = std::move(ref.get());
        input_bindings.apply(glapi, _prog, _attrib_bindings);
        glapi.get_uniform_location(_prog, "Camera") >> _camera_loc;
    }

    void handle_gl_geometry_and_bindings_loaded(
      identifier_t,
      std::reference_wrapper<geometry_and_bindings> ref,
      const url&) noexcept {
        _geom = std::move(ref.get());
    }

    execution_context& _ctx;
    video_context& _video;
    resource_loader& _loader;
    oglplus::vertex_attrib_bindings _attrib_bindings;
    geometry_and_bindings _geom;
    oglplus::owned_program_name _prog;
    background_color_depth _bg;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera _camera;
    oglplus::uniform_location _camera_loc;
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _loader{ec.loader()}
  , _attrib_bindings{shapes::vertex_attrib_kind::position, shapes::vertex_attrib_kind::normal}
  , _bg{0.4F, 0.F, 1.F} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    _loader.connect_observer(*this);

    _loader.request_gl_geometry_and_bindings(
      url{"json:///ShapeJson"}, _video, _attrib_bindings);
    _loader.request_gl_program(url{"json:///GLProgram"}, _video);
    _loader.request_gl_texture(
      url{"json:///OGLplusTex"}, _video, GL.texture_2d, GL.texture0);

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.1F)
      .set_orbit_max(3.5F)
      .set_fov(right_angle_());

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    gl.enable(GL.depth_test);
}
//------------------------------------------------------------------------------
void example_cube::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_cube::update() noexcept {
    _loader.update();
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state);
    }

    _bg.clear(_video);

    const auto& glapi = _video.gl_api();
    glapi.use_program(_prog);

    if(_camera.has_changed()) {
        glapi.set_uniform(_prog, _camera_loc, _camera.matrix(_video));
    }

    _geom.draw(glapi);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {
    const auto& glapi = _video.gl_api();
    glapi.clean_up(std::move(_prog));
    _geom.clean_up(_video);

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
    enable_message_bus(ctx);
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
