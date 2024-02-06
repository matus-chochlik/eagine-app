/// @example app/021_cel_shading/main.cpp
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
class example_cel : public timeouting_application {
public:
    example_cel(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_load_event(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_cel::_on_load_event>(this);
    }

    video_context& _video;

    orbiting_camera _camera;
    icosahedron_geometry _shape;
    cel_program _prog;
    background_icosahedron _bg;
};
//------------------------------------------------------------------------------
example_cel::example_cel(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _shape{context()}
  , _prog{context()}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.0F}, {0.4F, 0.4F, 0.4F, 0.0F}, 1.F} {
    _shape.load_event.connect(_load_handler());
    _prog.load_event.connect(_load_handler());

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(0.7F)
      .set_orbit_max(3.0F)
      .set_fov(right_angle_());

    _camera.connect_inputs(context()).basic_input_mapping(context());
    context().setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_cel::_on_load_event(const loaded_resource_base&) noexcept {
    if(_shape and _prog) {
        _prog.apply_input_bindings(_video, _shape);
        _prog.set_projection(_video, _camera);
        reset_timeout();
    }
}
//------------------------------------------------------------------------------
void example_cel::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 3.F);
    }

    _bg.clear(_video, _camera);

    if(_shape and _prog) {
        _prog.use(_video);
        _prog.set_projection(_video, _camera);
        _prog.set_modelview(context(), _video);
        _shape.use_and_draw(_video);
    } else {
        _shape.load_if_needed(context());
        _prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cel::clean_up() noexcept {

    _prog.clean_up(context());
    _shape.clean_up(context());
    _bg.clean_up(_video);

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
        return launch_with_video<example_cel>(ec);
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
