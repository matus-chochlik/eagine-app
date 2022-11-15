/// @example app/030_stencil_shadow/main.cpp
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

namespace eagine::app {
//------------------------------------------------------------------------------
class example_stencil_shadow : public timeouting_application {
public:
    example_stencil_shadow(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_loaded(const loaded_resource_base&) noexcept;
    void _on_prog_loaded(const gl_program_resource::load_info&) noexcept;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    puzzle_progress<6> _load_progress;
    video_context& _video;
    background_skybox _bg;

    gl_program_resource _draw_prog;
    gl_program_resource _mask_prog;
    gl_geometry_and_bindings_resource _wheelcart;
    gl_texture_resource _cube_tex;
    gl_texture_resource _color_tex;
    gl_texture_resource _light_tex;

    oglplus::uniform_location camera_draw_loc;
    oglplus::uniform_location camera_mask_loc;
    oglplus::uniform_location light_dir_draw_loc;
    oglplus::uniform_location light_dir_mask_loc;
    oglplus::uniform_location light_mult_loc;

    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_stencil_shadow::example_stencil_shadow(
  execution_context& ec,
  video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{90}}
  , _load_progress{ec.progress(), "Loading resources"}
  , _video{vc}
  , _bg{_video, 0}
  , _draw_prog{url{"json:///DrawProg"}, context()}
  , _mask_prog{url{"json:///MaskProg"}, context()}
  , _wheelcart{url{"json:///Wheelcart"}, context()}
  , _cube_tex{url{"json:///CloudyDay"}, context()}
  , _color_tex{url{"json:///CartColor"}, context()}
  , _light_tex{url{"json:///CartAOccl"}, context()} {
    _wheelcart.base_loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_loaded>(this));
    _draw_prog.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_prog_loaded>(this));
    _mask_prog.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_prog_loaded>(this));
    _cube_tex.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_tex_loaded>(this));
    _color_tex.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_tex_loaded>(this));
    _light_tex.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_tex_loaded>(this));

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);

    // camera
    camera.set_pitch_max(degrees_(89.F))
      .set_pitch_min(degrees_(10.F))
      .set_orbit_max(6.5F)
      .set_orbit_min(3.5F)
      .set_target(0.F)
      .set_fov(degrees_(70))
      .set_near(0.01F)
      .set_far(100.0F);
    camera.idle_update(context().state(), 11.F);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::_on_loaded(const loaded_resource_base&) noexcept {
    _load_progress.update_progress(
      _draw_prog && _mask_prog && _wheelcart && _color_tex && _light_tex &&
      _cube_tex);
}
//------------------------------------------------------------------------------
void example_stencil_shadow::_on_prog_loaded(
  const gl_program_resource::load_info& loaded) noexcept {
    if(loaded.resource.is(_draw_prog)) {
        loaded.apply_input_bindings(_wheelcart);
        loaded.set_uniform("ColorTex", 1);
        loaded.set_uniform("LightTex", 2);
        loaded.get_uniform_location("LightMult") >> light_mult_loc;
        loaded.get_uniform_location("LightDir") >> light_dir_draw_loc;
        loaded.get_uniform_location("Camera") >> camera_draw_loc;
    }
    if(loaded.resource.is(_mask_prog)) {
        loaded.apply_input_bindings(_wheelcart);
        loaded.get_uniform_location("LightDir") >> light_dir_mask_loc;
        loaded.get_uniform_location("Camera") >> camera_mask_loc;
    }
    _on_loaded(loaded.resource);
}
//------------------------------------------------------------------------------
void example_stencil_shadow::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL = _video.gl_api().constants();
    loaded.parameter_i(GL.texture_min_filter, GL.linear);
    loaded.parameter_i(GL.texture_mag_filter, GL.linear);
    loaded.parameter_i(GL.texture_wrap_s, GL.clamp_to_edge);
    loaded.parameter_i(GL.texture_wrap_t, GL.clamp_to_edge);
    _on_loaded(loaded.resource);
}
//------------------------------------------------------------------------------
void example_stencil_shadow::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 11.F);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    if(_load_progress.done()) {
        const auto ft = state.frame_time().value();
        const auto light_angle{turns_(ft * 0.1F)};
        const oglplus::vec3 light_dir{
          10.F * cos(light_angle), 10.F, 10.F * sin(light_angle)};

        // first color pass (shadow)
        gl.color_mask(GL.true_, GL.true_, GL.true_, GL.true_);
        gl.depth_mask(GL.true_);
        _bg.clear(_video, camera);
        gl.disable(GL.stencil_test);

        _wheelcart.use(_video);
        gl.use_program(_draw_prog);
        glapi.set_uniform(_draw_prog, light_mult_loc, 0.7F);
        glapi.set_uniform(_draw_prog, light_dir_draw_loc, light_dir);
        glapi.set_uniform(_draw_prog, camera_draw_loc, camera.matrix(_video));
        gl.cull_face(GL.back);
        _wheelcart.draw(_video);

        // stencil update pass
        gl.color_mask(GL.false_, GL.false_, GL.false_, GL.false_);
        gl.depth_mask(GL.false_);
        gl.clear(GL.depth_buffer_bit);
        gl.enable(GL.stencil_test);
        gl.stencil_func(GL.always, 0);
        gl.stencil_op_separate(GL.front, GL.keep, GL.keep, GL.incr);
        gl.stencil_op_separate(GL.back, GL.keep, GL.keep, GL.decr);

        gl.use_program(_mask_prog);
        glapi.set_uniform(_mask_prog, light_dir_mask_loc, light_dir);
        glapi.set_uniform(_mask_prog, camera_mask_loc, camera.matrix(_video));
        gl.cull_face(GL.back);
        _wheelcart.draw(_video);
        gl.cull_face(GL.front);
        _wheelcart.draw(_video);

        // second color pass (light)
        gl.color_mask(GL.true_, GL.true_, GL.true_, GL.true_);
        gl.depth_mask(GL.true_);
        gl.clear(GL.depth_buffer_bit);
        gl.stencil_func(GL.equal, 0);
        gl.stencil_op(GL.keep, GL.keep, GL.keep);

        gl.use_program(_draw_prog);
        glapi.set_uniform(_draw_prog, light_mult_loc, 1.0F);
        glapi.set_uniform(_draw_prog, light_dir_draw_loc, light_dir);
        glapi.set_uniform(_draw_prog, camera_draw_loc, camera.matrix(_video));
        gl.cull_face(GL.back);
        _wheelcart.draw(_video);
    } else {
        if(_wheelcart) {
            _draw_prog.load_if_needed(context());
            _mask_prog.load_if_needed(context());
        } else {
            _wheelcart.load_if_needed(context());
        }
        _cube_tex.load_if_needed(
          context(), GL.texture_cube_map, GL.texture0 + 0);
        _color_tex.load_if_needed(context(), GL.texture_2d, GL.texture0 + 1);
        _light_tex.load_if_needed(context(), GL.texture_2d, GL.texture0 + 2);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::clean_up() noexcept {
    _bg.clean_up(_video);
    _light_tex.clean_up(context());
    _color_tex.clean_up(context());
    _cube_tex.clean_up(context());
    _mask_prog.clean_up(context());
    _draw_prog.clean_up(context());
    _video.end();
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input();
        opts.require_video().with_stencil();
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
               gl.tex_image2d && GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_stencil_shadow>(ec, vc)};
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
