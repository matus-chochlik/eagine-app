/// @example app/031_tessellation/main.cpp
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
class example_sphere : public application {
public:
    example_sphere(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    background_icosahedron _bg;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;
    sphere_program prog;
    icosahedron_geometry shape;
};
//------------------------------------------------------------------------------
example_sphere::example_sphere(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _bg{_video, {0.0F, 0.0F, 0.0F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F} {

    prog.init(vc);
    shape.init(vc);

    prog.bind_position_location(vc, shape.position_loc());
    prog.bind_offsets_block(vc, shape.offsets_binding());

    camera.set_near(0.1F)
      .set_far(100.F)
      .set_orbit_min(13.0F)
      .set_orbit_max(24.0F)
      .set_fov(degrees_(70.F));
    prog.set_projection(vc, camera);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_sphere::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_sphere::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 7.F);
    }

    _bg.clear(_video, camera);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    prog.use(_video);
    prog.set_projection(_video, camera);
    shape.draw(_video);

    _video.commit();
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
               GL.vertex_shader && GL.tess_control_shader &&
               GL.tess_evaluation_shader && GL.vertex_shader &&
               GL.geometry_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_sphere>(ec, vc)};
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
