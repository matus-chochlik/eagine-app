/// @example application/025_recursive_cube/main.cpp
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
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    execution_context& _ctx;
    video_context& _video;

    cube_program _prog;
    cube_geometry _cube;
    cube_draw_buffers _bufs;

    timeout _is_done{std::chrono::seconds{30}};
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _prog{_ctx}
  , _cube{_ctx} {

    _bufs.init(_ctx);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.8F, 0.8F, 0.8F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_cube::_on_resource_loaded(const loaded_resource_base&) noexcept {
    if(_prog && _cube) {
        _prog.input_bindings.apply(_video.gl_api(), _prog, _cube);
    }
}
//------------------------------------------------------------------------------
void example_cube::on_video_resize() noexcept {
    _video.gl_api().viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_cube::update() noexcept {
    if(_ctx.state().is_active()) {
        _is_done.reset();
    }

    if(_prog && _cube) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        _prog.prepare_frame(_ctx);
        _prog.set_texture(_ctx, _bufs.front_tex_unit());

        // draw into texture
        gl.bind_framebuffer(GL.draw_framebuffer, _bufs.back_fbo());
        gl.viewport(_bufs.side(), _bufs.side());

        _prog.set_projection(
          _ctx,
          oglplus::matrix_perspective(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 5.F) *
            oglplus::matrix_translation(0.F, 0.F, -2.F));

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _cube.use_and_draw(_video);

        // draw on screen
        gl.bind_framebuffer(GL.draw_framebuffer, oglplus::default_framebuffer);
        gl.viewport[_video.surface_size()];

        const float h = 0.55F;
        const float w = h * _video.surface_aspect();
        _prog.set_projection(
          _ctx,
          oglplus::matrix_perspective(-w, +w, -h, +h, 1.F, 3.F) *
            oglplus::matrix_translation(0.F, 0.F, -2.F));

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _cube.use_and_draw(_video);
        // swap texture draw buffers
        _bufs.swap();
    } else {
        _prog.update(_ctx);
        _cube.update(_ctx);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {

    _bufs.clean_up(_ctx);
    _cube.clean_up(_ctx);
    _prog.clean_up(_ctx);

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
