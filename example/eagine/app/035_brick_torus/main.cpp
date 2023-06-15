/// @example app/035_brick_torus/main.cpp
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
class example_parallax : public timeouting_application {
public:
    example_parallax(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_parallax::_on_resource_loaded>(this);
    }

    void _apply_texture() noexcept;
    void _switch_texture(const input&) noexcept;

    puzzle_progress<4> _load_progress;

    video_context& _video;

    torus_program _prog;
    torus_geometry _torus;
    brick_texture _bricks;
    stone_texture _stones;

    orbiting_camera _camera;
    bool _use_stones_tex{false};
};
//------------------------------------------------------------------------------
example_parallax::example_parallax(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{90}}
  , _load_progress{ec.progress(), "Loading resources"}
  , _video{vc}
  , _prog{ec}
  , _torus{ec}
  , _bricks{ec}
  , _stones{ec} {
    if(ec.main_context().args().find("--stones")) {
        _use_stones_tex = true;
    }

    _prog.base_loaded.connect(_load_handler());
    _torus.base_loaded.connect(_load_handler());
    _bricks.base_loaded.connect(_load_handler());
    _stones.base_loaded.connect(_load_handler());

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.05F)
      .set_orbit_max(1.35F)
      .set_fov(degrees_(40.F));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs()
      .connect_input(
        {"Example", "ChngTexMap"},
        make_callable_ref<&example_parallax::_switch_texture>(this))
      .map_key({"Example", "ChngTexMap"}, "T")
      .switch_input_mapping();

    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    gl.clear_color(0.25F, 0.25F, 0.25F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.clip_distance0 + 0);
    gl.enable(GL.clip_distance0 + 1);
    gl.enable(GL.clip_distance0 + 2);
}
//------------------------------------------------------------------------------
void example_parallax::_on_resource_loaded(
  const loaded_resource_base&) noexcept {
    if(_prog and _torus) {
        _prog.apply_input_bindings(_video, _torus);
    }
    _apply_texture();
    _load_progress.update_progress(_prog and _torus and _bricks and _stones);
}
//------------------------------------------------------------------------------
void example_parallax::_apply_texture() noexcept {
    if(_prog and _bricks and _stones) {
        if(_use_stones_tex) {
            _prog.set_texture_map(_video, _stones.tex_unit());
        } else {
            _prog.set_texture_map(_video, _bricks.tex_unit());
        }
    }
}
//------------------------------------------------------------------------------
void example_parallax::_switch_texture(const input& i) noexcept {
    if(not i) {
        _use_stones_tex = not _use_stones_tex;
        _apply_texture();
        reset_timeout();
    }
}
//------------------------------------------------------------------------------
void example_parallax::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 17.F);
    }

    if(_load_progress.done()) {
        const auto rad = radians_(state.frame_time().value());
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _prog.set_camera(_video, _camera);
        _prog.set_light(
          _video,
          oglplus::vec3(cos(rad * -0.618F) * 8, sin(rad) * 7, cos(rad) * 6));

        _prog.set_model(
          _video,
          oglplus::matrix_translation(0.5F, 0.F, 0.F) *
            oglplus::matrix_rotation_x(right_angles_(-0.5F)));
        _torus.use_and_draw(_video);

        _prog.set_model(
          _video,
          oglplus::matrix_translation(-0.5F, 0.F, 0.F) *
            oglplus::matrix_rotation_x(right_angles_(+0.5F)));
        _torus.use_and_draw(_video);
    } else {
        _prog.load_if_needed(context());
        _torus.load_if_needed(context());
        _bricks.load_if_needed(context());
        _stones.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_parallax::clean_up() noexcept {
    _bricks.clean_up(context());
    _torus.clean_up(context());
    _prog.clean_up(context());

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
        return launch_with_video<example_parallax>(ec);
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
