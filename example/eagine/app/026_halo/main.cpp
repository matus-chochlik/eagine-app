/// @example app/026_halo/main.cpp
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
class example_halo : public timeouting_application {
public:
    example_halo(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_halo::_on_resource_loaded>(this);
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

    _surf_prog.base_loaded.connect(_load_handler());
    _halo_prog.base_loaded.connect(_load_handler());
    _shape.base_loaded.connect(_load_handler());

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
void example_halo::_on_resource_loaded(const loaded_resource_base&) noexcept {
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

    if(_surf_prog && _halo_prog && _shape) {
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

    _video.commit();
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

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable && gl.clear_color && gl.create_shader &&
               gl.shader_source && gl.compile_shader && gl.create_program &&
               gl.attach_shader && gl.link_program && gl.use_program &&
               gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
               gl.gen_vertex_arrays && gl.bind_vertex_array &&
               gl.get_attrib_location && gl.vertex_attrib_pointer &&
               gl.enable_vertex_attrib_array && gl.draw_arrays &&
               GL.vertex_shader && GL.geometry_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_halo>(ec, vc)};
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
