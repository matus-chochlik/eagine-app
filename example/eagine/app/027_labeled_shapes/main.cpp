/// @example app/027_labeled_shapes/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.guiplus;
import eagine.app;
import std;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_labels : public timeouting_application {
public:
    example_labels(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

    void update_overlays(guiplus::gui_utils& gui) noexcept final;

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;
    video_context& _video;

    draw_program _prog;

    example_shapes _shapes;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_labels::example_labels(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _prog{ec}
  , _shapes{ec} {

    _prog.loaded.connect(make_callable_ref<&example_labels::_on_loaded>(this));

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(3.0F)
      .set_orbit_max(4.0F)
      .set_fov(degrees_(70));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    gl.clear_color(0.45F, 0.45F, 0.45F, 1.0F);
    gl.clear_depth(1.F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_labels::_on_loaded(const gl_program_resource::load_info&) noexcept {
    if(_prog) {
        _prog.apply_input_bindings(_video, _shapes.front().geometry);
        _prog.use(_video);
    }
}
//------------------------------------------------------------------------------
void example_labels::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 3.F);
    }

    if(_prog and _shapes) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        camera_transforms camera{_camera, _video};
        _prog.set_projection(_video, camera);

        for(auto& shape : _shapes) {
            _prog.set_model(_video, shape.transform());

            shape.geometry.use(_video);
            shape.geometry.draw(_video);
            shape.screen_pos = camera.world_position_to_screen(shape.label_pos);
        }
    } else {
        _prog.load_if_needed(context());
        _shapes.load_if_needed(context());
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_labels::clean_up() noexcept {
    _shapes.clean_up(context());
    _video.end();
}
//------------------------------------------------------------------------------
void example_labels::update_overlays(guiplus::gui_utils& gui) noexcept {
    for(auto& shape : _shapes) {
        gui.text_overlay(shape.label, shape.screen_pos);
    }
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
        return launch_with_video<example_labels>(ec);
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
