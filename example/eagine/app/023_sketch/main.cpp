/// @example app/023_sketch/main.cpp
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
class example_sketch : public timeouting_application {
public:
    example_sketch(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    video_context& _video;
    background_icosahedron _bg;

    orbiting_camera _camera;
    sketch_program _sketch_prog;
    shape_geometry _shape;
    sketch_texture _tex;
};
//------------------------------------------------------------------------------
example_sketch::example_sketch(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _bg{_video, {0.50F, 0.50F, 0.45F, 1.0F}, {0.85F, 0.85F, 0.85F, 0.0F}, 1.F} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    _shape.init(vc);
    _tex.init(vc);

    _sketch_prog.init(vc);
    _sketch_prog.bind_position_location(vc, _shape.position_loc());
    _sketch_prog.bind_normal_location(vc, _shape.normal_loc());
    _sketch_prog.bind_coord_location(vc, _shape.wrap_coord_loc());

    // camera
    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(0.53F)
      .set_orbit_max(1.75F)
      .set_fov(degrees_(50.F));

    gl.clear_color(0.85F, 0.85F, 0.85F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
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

    _sketch_prog.prepare_frame(
      _video, _camera, context().state().frame_time().value());
    _shape.use_and_draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_sketch::clean_up() noexcept {

    _bg.clean_up(_video);
    _sketch_prog.clean_up(_video);
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
                    return {std::make_unique<example_sketch>(ec, vc)};
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
