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
class example_stencil_shadow : public application {
public:
    example_stencil_shadow(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_prog_loaded(const gl_program_resource::load_info&) noexcept;
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    execution_context& _ctx;
    video_context& _video;
    background_skybox _bg;

    gl_program_resource _draw_prog;
    gl_program_resource _mask_prog;
    gl_geometry_and_bindings_resource _wheelcart;

    oglplus::owned_texture_name color_tex{};
    oglplus::owned_texture_name light_tex{};

    oglplus::uniform_location camera_draw_loc;
    oglplus::uniform_location camera_mask_loc;
    oglplus::uniform_location light_dir_draw_loc;
    oglplus::uniform_location light_dir_mask_loc;
    oglplus::uniform_location light_mult_loc;

    orbiting_camera camera;
    timeout _is_done{std::chrono::seconds{90}};
};
//------------------------------------------------------------------------------
example_stencil_shadow::example_stencil_shadow(
  execution_context& ec,
  video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _bg{_video, embed<"CubeMap">("cloudy_day").unpack(ec)}
  , _draw_prog{url{"json:///DrawProg"}, _ctx}
  , _mask_prog{url{"json:///MaskProg"}, _ctx}
  , _wheelcart{url{"json:///Wheelcart"}, _ctx} {
    _draw_prog.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_prog_loaded>(this));
    _mask_prog.loaded.connect(
      make_callable_ref<&example_stencil_shadow::_on_prog_loaded>(this));

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // color texture
    const auto color_tex_src{embed<"ColorTex">("wheelcart_1_color")};

    gl.gen_textures() >> color_tex;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d, color_tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(color_tex_src.unpack(ec)));

    // light texture
    const auto light_tex_src{embed<"LightTex">("wheelcart_1_aoccl")};

    gl.gen_textures() >> light_tex;
    gl.active_texture(GL.texture0 + 1);
    gl.bind_texture(GL.texture_2d, light_tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(light_tex_src.unpack(ec)));

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
    camera.idle_update(_ctx.state(), 11.F);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::_on_prog_loaded(
  const gl_program_resource::load_info& loaded) noexcept {
    if(loaded.resource.is(_draw_prog)) {
        loaded.apply_input_bindings(_wheelcart);
        loaded.set_uniform("ColorTex", 0);
        loaded.set_uniform("LightTex", 1);
        loaded.get_uniform_location("LightMult") >> light_mult_loc;
        loaded.get_uniform_location("LightDir") >> light_dir_draw_loc;
        loaded.get_uniform_location("Camera") >> camera_draw_loc;
    }
    if(loaded.resource.is(_mask_prog)) {
        loaded.apply_input_bindings(_wheelcart);
        loaded.get_uniform_location("LightDir") >> light_dir_mask_loc;
        loaded.get_uniform_location("Camera") >> camera_mask_loc;
    }
}
//------------------------------------------------------------------------------
void example_stencil_shadow::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_stencil_shadow::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 11.F);
    }

    if(_draw_prog && _mask_prog && _wheelcart) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

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
    } else if(_wheelcart) {
        _draw_prog.update(_ctx);
        _mask_prog.update(_ctx);
    } else {
        _wheelcart.update(_ctx);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::clean_up() noexcept {
    _bg.clean_up(_video);
    _wheelcart.clean_up(_ctx);
    _mask_prog.clean_up(_ctx);
    _draw_prog.clean_up(_ctx);

    const auto& gl = _video.gl_api();
    gl.clean_up(std::move(light_tex));
    gl.clean_up(std::move(color_tex));

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
