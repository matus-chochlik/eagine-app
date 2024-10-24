/// @example app/018_uv_map_crate/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
import std;

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
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    video_context& _video;
    old_resource_manager _resources;
    managed_gl_program _prog;
    managed_gl_geometry_and_bindings _crate;
    managed_gl_texture _color_tex;
    managed_gl_texture _light_tex;

    oglplus::uniform_location _camera_loc;
    orbiting_camera camera;
};
//------------------------------------------------------------------------------
example_uv_map::example_uv_map(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _prog{_resources, ec, url{"json:///Program"}}
  , _crate{_resources, ec, url{"json:///Crate"}, oglplus::vertex_attrib_bindings{}, 0}
  , _color_tex{
      _resources,
      ec,
      url{"eagitex:///CrateColor"},
      _video.gl_api().texture_2d,
      _video.gl_api().texture0 + 0}
  , _light_tex{
      _resources,
      ec,
      url{"eagitex:///CrateLight"},
      _video.gl_api().texture_2d,
      _video.gl_api().texture0 + 1} {

    _prog.connect_to<&example_uv_map::_on_loaded>(this);
    _crate.connect_to<&example_uv_map::_on_loaded>(this);
    _crate.connect_to<&example_uv_map::_on_shp_loaded>(this);
    _color_tex.connect_to<&example_uv_map::_on_tex_loaded>(this);
    _light_tex.connect_to<&example_uv_map::_on_tex_loaded>(this);

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_uv_map::_on_loaded(const loaded_resource_base&) noexcept {
    if(_prog and _crate) {
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
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 2.0F)
      .set_orbit_max(sr * 4.0F)
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

    _video.commit(*this);
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

    auto check_requirements(video_context& vc) -> bool final {
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
      -> unique_holder<application> final {
        return launch_with_video<example_uv_map>(ec);
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad> {
    return {hold<example_launchpad>};
}
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
