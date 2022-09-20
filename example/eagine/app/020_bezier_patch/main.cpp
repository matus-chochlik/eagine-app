/// @example app/020_bezier_patch/main.cpp
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
class example_bpatch : public application {
public:
    example_bpatch(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;
    patch_program prog;
    patch_geometry shape;
};
//------------------------------------------------------------------------------
example_bpatch::example_bpatch(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    prog.init(ec, vc);
    shape.init(ec, vc);

    prog.bind_position_location(vc, shape.position_loc());

    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(4.0F)
      .set_orbit_max(12.0F)
      .set_fov(degrees_(70.F));
    prog.set_projection(vc, camera);

    gl.clear_color(0.35F, 0.35F, 0.35F, 0.0F);

    gl.disable(GL.depth_test);
    gl.patch_parameter_i(GL.patch_vertices, 16);
    gl.blend_func(GL.src_alpha, GL.one_minus_src_alpha);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_bpatch::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_bpatch::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 8.F);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit);

    prog.set_projection(_video, camera);

    gl.enable(GL.blend);
    gl.polygon_mode(GL.front_and_back, GL.fill);
    prog.set_surface_color(_video);
    shape.draw(_video);
    gl.disable(GL.blend);

    gl.polygon_mode(GL.front_and_back, GL.line);
    prog.set_wireframe_color(_video);
    shape.draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_bpatch::clean_up() noexcept {

    prog.clean_up(_video);
    shape.clean_up(_video);

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
               GL.geometry_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_bpatch>(ec, vc)};
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
