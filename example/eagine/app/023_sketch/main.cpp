/// @example app/023_sketch/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_sketch : public timeouting_application {
public:
    example_sketch(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_loaded(const loaded_resource_base&) noexcept;

    video_context& _video;
    background_icosahedron _bg;

    sketch_program _sketch_prog;
    shape_geometry _shape;
    sketch_texture _tex;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_sketch::example_sketch(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _bg{_video, {0.50F, 0.50F, 0.45F, 1.0F}, {0.85F, 0.85F, 0.85F, 0.0F}, 1.F}
  , _sketch_prog{ec} {
    _sketch_prog.base_loaded.connect(
      make_callable_ref<&example_sketch::_on_loaded>(this));

    _shape.init(vc);
    _tex.init(vc);

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(0.53F)
      .set_orbit_max(1.75F)
      .set_fov(degrees_(50.F));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.85F, 0.85F, 0.85F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_sketch::_on_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_sketch_prog)) {
        _sketch_prog.apply_input_bindings(_video, _shape);
    }
}
//------------------------------------------------------------------------------
void example_sketch::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 9.F);
    }

    _bg.clear(_video, _camera);
    if(_sketch_prog) {
        _sketch_prog.prepare_frame(
          _video, _camera, context().state().frame_time().value());
        _shape.use_and_draw(_video);
    } else {
        _sketch_prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_sketch::clean_up() noexcept {

    _bg.clean_up(_video);
    _sketch_prog.clean_up(context());
    _shape.clean_up(_video);
    _tex.clean_up(_video);

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
               GL.vertex_shader and GL.geometry_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_sketch>(ec);
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
