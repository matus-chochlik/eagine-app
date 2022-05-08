/// @example app/035_brick_torus/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/timeout.hpp>

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_parallax : public application {
public:
    example_parallax(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{60}};

    orbiting_camera camera;
    torus_program prog;
    torus_geometry torus;
    torus_textures textures;
};
//------------------------------------------------------------------------------
example_parallax::example_parallax(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    prog.init(ec, vc);
    torus.init(ec, vc);
    textures.init(ec, vc);

    prog.bind_position_location(vc, torus.position_loc());
    prog.bind_normal_location(vc, torus.normal_loc());
    prog.bind_tangent_location(vc, torus.tangent_loc());
    prog.bind_texcoord_location(vc, torus.wrap_coord_loc());

    if(ec.main_context().args().find("--stones")) {
        prog.set_texture_map(vc, textures.stones_map_unit());
    } else {
        prog.set_texture_map(vc, textures.bricks_map_unit());
    }

    // camera
    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.05F)
      .set_orbit_max(1.35F)
      .set_fov(degrees_(40.F));

    gl.clear_color(0.25F, 0.25F, 0.25F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.clip_distance0 + 0);
    gl.enable(GL.clip_distance0 + 1);
    gl.enable(GL.clip_distance0 + 2);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_parallax::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_parallax::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 17.F);
    }

    const auto rad = radians_(state.frame_time().value());
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
    prog.set_camera(_video, camera);
    prog.set_light(
      _video,
      oglplus::vec3(cos(rad * -0.618F) * 8, sin(rad) * 7, cos(rad) * 6));

    prog.set_model(
      _video,
      oglplus::matrix_translation(0.5F, 0.F, 0.F) *
        oglplus::matrix_rotation_x(right_angles_(-0.5F)));
    torus.use_and_draw(_video);

    prog.set_model(
      _video,
      oglplus::matrix_translation(-0.5F, 0.F, 0.F) *
        oglplus::matrix_rotation_x(right_angles_(+0.5F)));
    torus.use_and_draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_parallax::clean_up() noexcept {

    textures.clean_up(_video);
    torus.clean_up(_video);
    prog.clean_up(_video);

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
                    return {std::make_unique<example_parallax>(ec, vc)};
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
