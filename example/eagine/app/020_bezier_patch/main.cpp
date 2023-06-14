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
class example_bpatch : public timeouting_application {
public:
    example_bpatch(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_prog_loaded(const gl_program_resource::load_info&) noexcept;

    video_context& _video;

    patch_program _prog;
    patch_geometry _shape;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_bpatch::example_bpatch(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _prog{context()} {
    _prog.loaded.connect(
      make_callable_ref<&example_bpatch::_on_prog_loaded>(this));

    _shape.init(context());

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(4.0F)
      .set_orbit_max(12.0F)
      .set_fov(degrees_(70.F));
    _prog.set_projection(vc, _camera);

    _camera.connect_inputs(ec).basic_input_mapping(context());
    context().setup_inputs().switch_input_mapping();

    const auto& [gl, GL] = _video.gl_api();

    gl.clear_color(0.35F, 0.35F, 0.35F, 0.0F);

    gl.disable(GL.depth_test);
    gl.patch_parameter_i(GL.patch_vertices, 16);
    gl.blend_func(GL.src_alpha, GL.one_minus_src_alpha);
}
//------------------------------------------------------------------------------
void example_bpatch::_on_prog_loaded(
  const gl_program_resource::load_info&) noexcept {
    _prog.bind_position_location(_video, _shape.position_loc());
}
//------------------------------------------------------------------------------
void example_bpatch::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 8.F);
    }

    if(_prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit);

        _prog.set_projection(_video, _camera);

        gl.enable(GL.blend);
        gl.polygon_mode(GL.front_and_back, GL.fill);
        _prog.set_surface_color(_video);
        _shape.draw(context());
        gl.disable(GL.blend);

        gl.polygon_mode(GL.front_and_back, GL.line);
        _prog.set_wireframe_color(_video);
        _shape.draw(context());
    } else {
        _prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_bpatch::clean_up() noexcept {

    _prog.clean_up(context());
    _shape.clean_up(context());

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
               GL.vertex_shader and GL.tess_control_shader and
               GL.tess_evaluation_shader and GL.vertex_shader and
               GL.geometry_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_bpatch>(ec);
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
