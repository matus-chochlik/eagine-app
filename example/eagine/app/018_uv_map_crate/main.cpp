/// @example app/018_uv_map_crate/main.cpp
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
class example_uv_map : public timeouting_application {
public:
    example_uv_map(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_loaded(const loaded_resource_base&) noexcept;

    void _on_shp_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;

    video_context& _video;
    resource_manager _resources;
    managed_gl_program _prog;
    managed_gl_geometry_and_bindings _crate;

    oglplus::owned_texture_name color_tex{};
    oglplus::owned_texture_name light_tex{};

    oglplus::uniform_location _camera_loc;
    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_uv_map::example_uv_map(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _resources{ec}
  , _prog{_resources, url{"json:///Program"}, _video}
  , _crate{_resources, url{"json:///Crate"}, _video, 0} {
    _prog.connect_to<&example_uv_map::_on_loaded>(this);
    _crate.connect_to<&example_uv_map::_on_loaded>(this);
    _crate.connect_to<&example_uv_map::_on_shp_loaded>(this);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // color texture
    const auto color_tex_src{embed<"ColorTex">("crate_2_color")};

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
    const auto light_tex_src{embed<"LightTex">("crate_2_light")};

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

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_uv_map::_on_loaded(const loaded_resource_base&) noexcept {
    if(_prog && _crate) {
        _prog->apply_input_bindings(_video, _crate);
        _prog->get_uniform_location(_video, "Camera") >> _camera_loc;
        _prog->set_uniform(_video, "ColorTex", 0);
        _prog->set_uniform(_video, "LightTex", 1);
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_uv_map::_on_shp_loaded(
  const gl_geometry_and_bindings_resource::load_info& loaded) noexcept {
    const auto bs = loaded.base.shape.bounding_sphere();
    const auto sr = bs.radius();
    camera.set_target(bs.center())
      .set_near(sr * 0.1F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 2.4F)
      .set_fov(degrees_(70));
}
//------------------------------------------------------------------------------
void example_uv_map::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 3.F);
    }

    if(_resources.are_loaded()) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _prog->set(glapi, _camera_loc, camera.matrix(_video));
        _crate->draw(glapi);
    } else {
        _resources.load();
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_uv_map::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_textures(std::move(light_tex));
    gl.delete_textures(std::move(color_tex));

    _resources.clean_up();
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
               gl.tex_image2d && GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_uv_map>(ec, vc)};
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
