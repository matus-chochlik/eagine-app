/// @example app/021_cel_shading/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/background/icosahedron.hpp>
#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/timeout.hpp>

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_cel : public application {
public:
    example_cel(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    background_icosahedron _bg;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;
    cel_program prog;
    icosahedron_geometry shape;
};
//------------------------------------------------------------------------------
example_cel::example_cel(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.0F}, {0.4F, 0.4F, 0.4F, 0.0F}, 1.F} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    prog.init(ec, vc);
    shape.init(vc);

    prog.bind_position_location(vc, shape.position_loc());

    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.3F)
      .set_orbit_max(6.0F)
      .set_fov(right_angle_());
    prog.set_projection(vc, camera);

    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_cel::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_cel::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state);
    }

    _bg.clear(_video, camera);

    prog.use(_video);
    prog.set_projection(_video, camera);
    prog.set_modelview(_ctx, _video);
    shape.use_and_draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cel::clean_up() noexcept {

    prog.clean_up(_video);
    shape.clean_up(_video);
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
} // namespace eagine::app
