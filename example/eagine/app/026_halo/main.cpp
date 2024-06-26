/// @example app/026_halo/main.cpp
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
class example_halo : public timeouting_application {
public:
    example_halo(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_load_event(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_halo::_on_load_event>(this);
    }

    video_context& _video;

    surface_program _surf_prog;
    halo_program _halo_prog;
    shape_geometry _shape;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_halo::example_halo(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _surf_prog{context()}
  , _halo_prog{context()}
  , _shape{context()} {

    _surf_prog.load_event.connect(_load_handler());
    _halo_prog.load_event.connect(_load_handler());
    _shape.load_event.connect(_load_handler());

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.4F)
      .set_orbit_max(3.5F)
      .set_fov(degrees_(70));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.25F, 0.25F, 0.25F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
    gl.blend_func(GL.src_alpha, GL.one);
}
//------------------------------------------------------------------------------
void example_halo::_on_load_event(const loaded_resource_base&) noexcept {
    if(_shape) {
        if(_surf_prog) {
            _surf_prog.use(_video);
            _surf_prog.apply_input_bindings(_video, _shape);
        }
        if(_halo_prog) {
            _halo_prog.use(_video);
            _halo_prog.apply_input_bindings(_video, _shape);
        }
    }
}
//------------------------------------------------------------------------------
void example_halo::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 11.F);
    }

    if(_surf_prog and _halo_prog and _shape) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);

        float t = context().state().frame_time().value();
        _surf_prog.prepare_frame(_video, _camera, t);
        _shape.draw(_video);

        gl.depth_mask(GL.false_);
        gl.enable(GL.blend);
        _halo_prog.prepare_frame(_video, _camera, t);
        _shape.draw(_video);
        gl.disable(GL.blend);
        gl.depth_mask(GL.true_);
    } else {
        _surf_prog.load_if_needed(context());
        _halo_prog.load_if_needed(context());
        _shape.load_if_needed(context());
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_halo::clean_up() noexcept {
    _surf_prog.clean_up(context());
    _halo_prog.clean_up(context());
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
               GL.vertex_shader and GL.geometry_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_halo>(ec);
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
