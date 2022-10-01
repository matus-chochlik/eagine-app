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
    void _handle_stream_data(
      identifier_t request_id,
      const span_size_t,
      const memory::span<const memory::const_block> data,
      const msgbus::blob_info&) noexcept {
        _ctx.cio_print("request ${requestId} data: (${data})")
          .arg("requestId", request_id)
          .arg("data", data.size());
    }

    void _handle_glsl_source(
      identifier_t request_id,
      const oglplus::glsl_source_ref& src,
      const url& locator) noexcept {
        _ctx.cio_print("request ${requestId}: ${locator} (${parts})")
          .arg("requestId", request_id)
          .arg("locator", locator.str())
          .arg("parts", src.count());
    }

    execution_context& _ctx;
    video_context& _video;
    resource_loader _loader;
    background_color_depth _bg;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _loader{ec.main_context()}
  , _bg{0.4F, 0.F, 1.F} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    connect<&example_cube::_handle_stream_data>(
      this, _loader.blob_stream_data_appended);
    connect<&example_cube::_handle_glsl_source>(
      this, _loader.shader_source_loaded);

    _loader.request_gl_shader_source(
      url{"glsl:///VertShader?kind=shader_type=vertex"});
    _loader.request_gl_shader_source(
      url{"glsl:///FragShader?shader_type=fragment"});

    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.1F)
      .set_orbit_max(3.5F)
      .set_fov(right_angle_());

    camera.connect_inputs(ec).basic_input_mapping(ec);
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
        camera.idle_update(state);
    }

    _bg.clear(_video);

    if(camera.has_changed()) {
        // glapi.set_uniform(prog, camera_loc, camera.matrix(_video));
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {

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
