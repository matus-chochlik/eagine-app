/// @example app/018_uv_map_hydrant/main.cpp
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
    auto _load_handler() noexcept {
        return make_callable_ref<&example_uv_map::_on_loaded>(this);
    }

    void _on_shp_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;
    auto _load_shp_handler() noexcept {
        return make_callable_ref<&example_uv_map::_on_shp_loaded>(this);
    }

    void _on_prg_loaded(const gl_program_resource::load_info&) noexcept;
    auto _load_prg_handler() noexcept {
        return make_callable_ref<&example_uv_map::_on_prg_loaded>(this);
    }

    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;
    auto _load_tex_handler() noexcept {
        return make_callable_ref<&example_uv_map::_on_tex_loaded>(this);
    }

    video_context& _video;
    resource_loader& _loader;
    gl_geometry_and_bindings_resource _shape;
    gl_program_resource _prog;
    gl_texture_resource _tex;
    oglplus::program_input_bindings _prog_inputs;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _light_dir_loc;

    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_uv_map::example_uv_map(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _loader{context().loader()}
  , _shape{url{"json:///HydrntShpe"}, _video, _loader}
  , _prog{url{"json:///HydrntProg"}, _video, _loader}
  , _tex{url{"json:///HydrantTex"}, _video, _loader} {
    _shape.base_loaded.connect(_load_handler());
    _shape.loaded.connect(_load_shp_handler());
    _prog.base_loaded.connect(_load_handler());
    _prog.loaded.connect(_load_prg_handler());
    _tex.base_loaded.connect(_load_handler());
    _tex.loaded.connect(_load_tex_handler());

    camera.set_pitch_max(degrees_(89.F))
      .set_pitch_min(degrees_(-1.F))
      .set_fov(degrees_(70));

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
}
//------------------------------------------------------------------------------
void example_uv_map::_on_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is_one_of(_shape, _prog)) {
        if(_shape && _prog) {
            _prog_inputs.apply(_video.gl_api(), _prog, _shape);
        }
    }
}
//------------------------------------------------------------------------------
void example_uv_map::_on_shp_loaded(
  const gl_geometry_and_bindings_resource::load_info& loaded) noexcept {
    const auto bs = loaded.base.shape.bounding_sphere();
    const auto sr = bs.radius();

    camera.set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 2.9F);
}
//------------------------------------------------------------------------------
void example_uv_map::_on_prg_loaded(
  const gl_program_resource::load_info& loaded) noexcept {
    _prog_inputs = loaded.base.input_bindings;
    loaded.set_uniform("Tex", 0);
    loaded.get_uniform_location("Camera") >> _camera_loc;
    loaded.get_uniform_location("LightDir") >> _light_dir_loc;
}
//------------------------------------------------------------------------------
void example_uv_map::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& [gl, GL] = loaded.base.gl_api();
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_t, GL.clamp_to_border);
}
//------------------------------------------------------------------------------
void example_uv_map::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 5.F);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;
    if(_shape && _prog && _tex) {
        const auto rad = radians_(state.frame_time().value());

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        if(camera.has_changed()) {
            glapi.set_uniform(_prog, _camera_loc, camera.matrix(_video));
        }
        glapi.set_uniform(
          _prog,
          _light_dir_loc,
          math::normalized(
            oglplus::vec3(cos(rad) * 6, sin(rad) * 7, sin(rad * 0.618F) * 8)));

        _shape.draw(_video);
    } else {
        _shape.load_if_needed(_video, _loader);
        _prog.load_if_needed(_video, _loader);
        _tex.load_if_needed(_video, _loader, GL.texture_2d_array, GL.texture0);
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_uv_map::clean_up() noexcept {
    _tex.clean_up(_video, _loader);
    _prog.clean_up(_video, _loader);
    _shape.clean_up(_video, _loader);
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
