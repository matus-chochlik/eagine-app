/// @example app/018_uv_map_barrel/main.cpp
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
    video_context& _video;
    void _on_loaded(const loaded_resource_base&) noexcept;

    void _on_shp_loaded(
      const gl_geometry_and_bindings_resource::load_info&) noexcept;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    resource_manager _resources;
    managed_gl_program _prog;
    managed_gl_geometry_and_bindings _barrel;
    managed_gl_texture _color_tex;
    managed_gl_texture _aoccl_tex;
    managed_gl_texture _rough_tex;

    oglplus::uniform_location _camera_loc;
    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_uv_map::example_uv_map(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _resources{ec}
  , _prog{_resources, url{"json:///Program"}, _video}
  , _barrel{_resources, url{"json:///Barrel"}, _video, 0} 
  , _color_tex{
      _resources,
	  url{"eagitex:///BarelColor"},
	  _video,
	  _video.gl_api().texture_2d,
	  _video.gl_api().texture0 + 0}
  , _aoccl_tex{
      _resources,
      url{"eagitex:///BarelAOccl"},
      _video,
      _video.gl_api().texture_2d,
      _video.gl_api().texture0 + 1}
  , _rough_tex{
     _resources,
     url{"eagitex:///BarelRough"},
     _video,
     _video.gl_api().texture_2d,
     _video.gl_api().texture0 + 2} {

    _prog.connect_to<&example_uv_map::_on_loaded>(this);
    _barrel.connect_to<&example_uv_map::_on_loaded>(this);
    _barrel.connect_to<&example_uv_map::_on_shp_loaded>(this);
    _color_tex.connect_to<&example_uv_map::_on_tex_loaded>(this);
    _aoccl_tex.connect_to<&example_uv_map::_on_tex_loaded>(this);
    _rough_tex.connect_to<&example_uv_map::_on_tex_loaded>(this);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_uv_map::_on_loaded(const loaded_resource_base&) noexcept {
    if(_prog and _barrel) {
        _prog->apply_input_bindings(_video, _barrel);
        _prog->get_uniform_location(_video, "Camera") >> _camera_loc;
        _prog->set_uniform(_video, "ColorTex", 0);
        _prog->set_uniform(_video, "LightTex", 1);
        _prog->set_uniform(_video, "RoughTex", 2);
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_uv_map::_on_shp_loaded(
  const gl_geometry_and_bindings_resource::load_info& loaded) noexcept {
    const auto bs = loaded.base.shape.bounding_sphere();
    const auto sr = bs.radius();
    _camera.set_target(bs.center())
      .set_near(sr * 0.1F)
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 1.8F)
      .set_orbit_max(sr * 3.6F)
      .set_fov(degrees_(70));
}
//------------------------------------------------------------------------------
void example_uv_map::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL = _video.gl_api().constants();
    loaded.parameter_i(GL.texture_min_filter, GL.linear);
    loaded.parameter_i(GL.texture_mag_filter, GL.linear);
    loaded.parameter_i(GL.texture_wrap_s, GL.clamp_to_border);
    loaded.parameter_i(GL.texture_wrap_t, GL.clamp_to_border);
}
//------------------------------------------------------------------------------
void example_uv_map::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 5.F);
    }

    if(_resources.are_loaded()) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;
        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);

        _prog->set(glapi, _camera_loc, _camera.matrix(_video));
        _barrel->draw(glapi);
    } else {
        _resources.load();
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_uv_map::clean_up() noexcept {
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

        return gl.disable and gl.clear_color and gl.create_shader and
               gl.shader_source and gl.compile_shader and gl.create_program and
               gl.attach_shader and gl.link_program and gl.use_program and
               gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
               gl.gen_vertex_arrays and gl.bind_vertex_array and
               gl.get_attrib_location and gl.vertex_attrib_pointer and
               gl.enable_vertex_attrib_array and gl.draw_arrays and
               gl.tex_image2d and GL.vertex_shader and GL.fragment_shader;
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
