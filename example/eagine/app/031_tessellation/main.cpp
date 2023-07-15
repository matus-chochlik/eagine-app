/// @example app/031_tessellation/main.cpp
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
class example_sphere : public timeouting_application {
public:
    example_sphere(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_prog_loaded(const gl_program_resource::load_info&) noexcept;

    video_context& _video;
    background_icosahedron _bg;

    sphere_program _prog;
    icosahedron_geometry _shape;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_sphere::example_sphere(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _bg{_video, {0.0F, 0.0F, 0.0F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _prog{ec} {
    _prog.loaded.connect(
      make_callable_ref<&example_sphere::_on_prog_loaded>(this));

    _shape.init(vc);

    _camera.set_near(0.1F)
      .set_far(100.F)
      .set_orbit_min(13.0F)
      .set_orbit_max(24.0F)
      .set_fov(degrees_(70.F));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_sphere::_on_prog_loaded(
  const gl_program_resource::load_info& loaded) noexcept {
    _prog.apply_input_bindings(_video, _shape.attrib_bindings());
    loaded.shader_storage_block_binding(
      "OffsetBlock", _shape.offsets_binding());
}
//------------------------------------------------------------------------------
void example_sphere::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 7.F);
    }

    _bg.clear(_video, _camera);
    if(_prog) {

        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.enable(GL.depth_test);
        gl.enable(GL.cull_face);
        gl.cull_face(GL.back);

        _prog.use(_video);
        _prog.set_projection(_video, _camera);
        _shape.draw(_video);
    } else {
        _prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_sphere::clean_up() noexcept {
    _prog.clean_up(context());
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
        return launch_with_video<example_sphere>(ec);
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
