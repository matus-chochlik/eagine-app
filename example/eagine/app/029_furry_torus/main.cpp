/// @example app/029_furry_torus/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_fur : public timeouting_application {
public:
    example_fur(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_loaded(const loaded_resource_base&) noexcept;

    video_context& _video;
    background_icosahedron _bg;

    oglplus::tmat<float, 4, 4, true> prev_model;

    hair_program _hair_prog;
    surface_program _surf_prog;
    shape_textures _shape_tex;
    shape_surface _surf;
    shape_hair _hair;

    orbiting_camera _camera;
    bool _use_monkey_shape{false};
};
//------------------------------------------------------------------------------
example_fur::example_fur(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{120}}
  , _video{vc}
  , _bg{_video, {0.1F, 0.1F, 0.1F, 1.F}, {0.35F, 0.40F, 0.30F, 0.0F}, 1.F}
  , _hair_prog{ec}
  , _surf_prog{ec} {
    _hair_prog.base_loaded.connect(
      make_callable_ref<&example_fur::_on_loaded>(this));
    _surf_prog.base_loaded.connect(
      make_callable_ref<&example_fur::_on_loaded>(this));

    _use_monkey_shape = ec.main_context().args().find("--monkey");

    const auto gen = [&]() -> std::shared_ptr<shapes::generator> {
        if(_use_monkey_shape) {
            const auto json_text =
              as_chars(embed<"MonkeyJson">("monkey.json").unpack(ec));
            return shapes::from_value_tree(
              valtree::from_json_text(json_text, ec.main_context()),
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

    _shape_tex.init(_video);
    _surf.init(_video, gen);
    _hair.init(_video, gen);

    prev_model = oglplus::matrix_rotation_y(radians_(0.F)) *
                 oglplus::matrix_rotation_x(radians_(0.F));

    const auto bs = gen->bounding_sphere();
    const auto sr = bs.radius();
    _camera.set_target(bs.center())
      .set_near(0.1F * sr)
      .set_far(50.F * sr)
      .set_orbit_min(1.2F * sr)
      .set_orbit_max(2.2F * sr)
      .set_fov(degrees_(45));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_fur::_on_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_surf_prog)) {
        _surf_prog.apply_input_bindings(_video, _surf);
        if(_use_monkey_shape) {
            _surf_prog.set_texture(_video, _shape_tex.map_unit_monkey());
        } else {
            _surf_prog.set_texture(_video, _shape_tex.map_unit_zebra());
        }
    }
    if(loaded.is(_hair_prog)) {
        _hair_prog.apply_input_bindings(_video, _hair);
        if(_use_monkey_shape) {
            _hair_prog.set_texture(_video, _shape_tex.map_unit_monkey());
        } else {
            _hair_prog.set_texture(_video, _shape_tex.map_unit_zebra());
        }
    }
}
//------------------------------------------------------------------------------
void example_fur::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 11.F);
    }

    const auto t = state.frame_time().value();
    const auto model = oglplus::matrix_rotation_y(degrees_(-t * 23.F)) *
                       oglplus::matrix_rotation_x(degrees_(t * 41.F));

    if(_surf_prog && _hair_prog) {
        _bg.clear(_video, _camera);

        _surf_prog.use(_video);
        _surf_prog.set_projection(_video, _camera);
        _surf_prog.set_model(_video, model);
        _surf.use_and_draw(_video);

        _hair_prog.use(_video);
        _hair_prog.set_projection(_video, _camera);
        _hair_prog.set_model(_video, prev_model, model);
        _hair.use_and_draw(_video);
    } else {
        _surf_prog.load_if_needed(context());
        _hair_prog.load_if_needed(context());
    }

    prev_model = model;

    _video.commit();
}
//------------------------------------------------------------------------------
void example_fur::clean_up() noexcept {
    _shape_tex.clean_up(_video);
    _surf_prog.clean_up(context());
    _hair_prog.clean_up(context());
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
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
