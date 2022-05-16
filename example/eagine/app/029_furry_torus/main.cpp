/// @example app/029_furry_torus/main.cpp
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
#include <eagine/embed.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/shapes/scaled_wrap_coords.hpp>
#include <eagine/shapes/torus.hpp>
#include <eagine/shapes/value_tree.hpp>
#include <eagine/timeout.hpp>
#include <eagine/value_tree/json.hpp>

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_fur : public application {
public:
    example_fur(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    background_icosahedron _bg;
    timeout _is_done{std::chrono::seconds{120}};

    oglplus::tmat<float, 4, 4, true> prev_model;

    orbiting_camera camera;
    shape_textures shape_tex;
    surface_program surf_prog;
    hair_program hair_prog;
    shape_surface surf;
    shape_hair hair;
};
//------------------------------------------------------------------------------
example_fur::example_fur(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.F}, {0.35F, 0.40F, 0.30F, 0.0F}, 1.F} {

    const bool use_monkey_shape = ec.main_context().args().find("--monkey");

    const auto gen = [&]() -> std::shared_ptr<shapes::generator> {
        if(use_monkey_shape) {
            const auto json_text =
              as_chars(embed(EAGINE_ID(MonkeyJson), "monkey.json").unpack(ec));
            return shapes::from_value_tree(
              valtree::from_json_text(json_text, ec.as_parent()),
              ec.as_parent());
        }
        return shapes::scale_wrap_coords(
          shapes::unit_torus(
            shapes::vertex_attrib_kind::position |
              shapes::vertex_attrib_kind::normal |
              shapes::vertex_attrib_kind::wrap_coord |
              shapes::vertex_attrib_kind::occlusion,
            18,
            36,
            0.6F),
          4.F,
          2.F,
          1.F);
    }();

    shape_tex.init(vc);
    surf_prog.init(vc);
    hair_prog.init(vc);
    surf.init(vc, gen);
    hair.init(vc, gen);

    surf_prog.use(vc);
    surf_prog.bind_position_location(vc, surf.position_loc());
    surf_prog.bind_texcoord_location(vc, surf.wrap_coord_loc());
    surf_prog.bind_occlusion_location(vc, surf.occlusion_loc());
    if(use_monkey_shape) {
        surf_prog.set_texture(vc, shape_tex.map_unit_monkey());
    } else {
        surf_prog.set_texture(vc, shape_tex.map_unit_zebra());
    }

    hair_prog.use(vc);
    hair_prog.bind_position_location(vc, hair.position_loc());
    hair_prog.bind_normal_location(vc, hair.normal_loc());
    hair_prog.bind_texcoord_location(vc, hair.wrap_coord_loc());
    hair_prog.bind_occlusion_location(vc, hair.occlusion_loc());
    if(use_monkey_shape) {
        hair_prog.set_texture(vc, shape_tex.map_unit_monkey());
    } else {
        hair_prog.set_texture(vc, shape_tex.map_unit_zebra());
    }

    prev_model = oglplus::matrix_rotation_y(radians_(0.F)) *
                 oglplus::matrix_rotation_x(radians_(0.F));

    // camera
    const auto bs = gen->bounding_sphere();
    const auto sr = bs.radius();
    camera.set_target(bs.center())
      .set_near(0.1F * sr)
      .set_far(50.F * sr)
      .set_orbit_min(1.2F * sr)
      .set_orbit_max(2.2F * sr)
      .set_fov(degrees_(45));

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_fur::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_fur::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 11.F);
    }

    const auto t = state.frame_time().value();
    const auto model = oglplus::matrix_rotation_y(degrees_(-t * 23.F)) *
                       oglplus::matrix_rotation_x(degrees_(t * 41.F));

    _bg.clear(_video, camera);

    surf_prog.use(_video);
    surf_prog.set_projection(_video, camera);
    surf_prog.set_model(_video, model);
    surf.use_and_draw(_video);

    hair_prog.use(_video);
    hair_prog.set_projection(_video, camera);
    hair_prog.set_model(_video, prev_model, model);
    hair.use_and_draw(_video);

    prev_model = model;

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
               GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_fur>(ec, vc)};
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
