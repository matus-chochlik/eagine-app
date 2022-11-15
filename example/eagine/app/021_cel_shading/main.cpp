/// @example app/021_cel_shading/main.cpp
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
class example_cel : public timeouting_application {
public:
    example_cel(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_cel::_on_resource_loaded>(this);
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
    _shape.base_loaded.connect(_load_handler());
    _prog.base_loaded.connect(_load_handler());

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.3F)
      .set_orbit_max(6.0F)
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
void example_cel::_on_resource_loaded(const loaded_resource_base&) noexcept {
    if(_shape && _prog) {
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
        _camera.idle_update(state);
    }

    _bg.clear(_video, _camera);

    if(_shape && _prog) {
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
                    return {std::make_unique<example_cel>(ec, vc)};
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
