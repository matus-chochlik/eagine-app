/// @example app/013_tiling/main.cpp
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
using resource_manager = basic_resource_manager<
  gl_geometry_and_bindings_resource,
  gl_shader_resource,
  gl_program_resource,
  gl_texture_resource,
  gl_buffer_resource>;

using managed_gl_geometry_and_bindings =
  managed_resource<gl_geometry_and_bindings>;
using managed_gl_program = managed_resource<oglplus::owned_program_name>;
using managed_gl_texture = managed_resource<oglplus::owned_texture_name>;
//------------------------------------------------------------------------------
class example_tiling : public timeouting_application {
public:
    example_tiling(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;
    void change_tileset(const input&) noexcept;

private:
    static auto _prog_url() {
        return url{"json:///Program"};
    }
    static auto _cube_url() {
        return url{
          "shape:///unit_round_cube?"
          "position=true+normal=true+face_coord=true+divisions=32"};
    }
    static auto _tiling_tex_url() {
        return url{"json:///TilingTex"};
    }

    void _on_loaded(const loaded_resource_base&) noexcept;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    video_context& _video;
    resource_manager _resources;
    managed_gl_program _prog;
    managed_gl_geometry_and_bindings _cube;
    managed_gl_texture _tiling_tex;
    managed_gl_texture _tiles1_tex;
    managed_gl_texture _tiles2_tex;
    managed_gl_texture _tiles3_tex;
    managed_gl_texture _tiles4_tex;

    std::array<int, 4> tileset_tex_units{1, 2, 3, 4};
    std::size_t tileset_tex_idx{0U};

    animated_value<std::tuple<radians_t<float>, radians_t<float>, float>, float>
      geo_coord;
    enum class animation_status { zoom_in, zoom_out, relocate };
    animation_status camera_status{animation_status::zoom_in};

    orbiting_camera camera;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _tileset_tex_loc;
};
//------------------------------------------------------------------------------
example_tiling::example_tiling(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _resources{ec}
  , _prog{_resources, _prog_url(), _video}
  , _cube{_resources, _cube_url(), _video, 0}
  , _tiling_tex{
      _resources,
      _tiling_tex_url(),
      _video,
      _video.gl_api().texture_2d_array,
      _video.gl_api().texture0}
  , _tiles1_tex{
      _resources,
      url{"json:///NodesTex"},
      _video,
      _video.gl_api().texture_2d_array,
      _video.gl_api().texture0+1}
  , _tiles2_tex{
      _resources,
      url{"json:///CnnctsTex"},
      _video,
      _video.gl_api().texture_2d_array,
      _video.gl_api().texture0+2}
  , _tiles3_tex{
      _resources,
      url{"json:///BlocksTex"},
      _video,
      _video.gl_api().texture_2d_array,
      _video.gl_api().texture0+3}
  , _tiles4_tex{
      _resources,
      url{"json:///ThicketTex"},
      _video,
      _video.gl_api().texture_2d_array,
      _video.gl_api().texture0+4} {

    _prog.connect_to<&example_tiling::_on_loaded>(this);
    _cube.connect_to<&example_tiling::_on_loaded>(this);

    _tiling_tex.connect_to<&example_tiling::_on_tex_loaded>(this);
    _tiles1_tex.connect_to<&example_tiling::_on_tex_loaded>(this);
    _tiles2_tex.connect_to<&example_tiling::_on_tex_loaded>(this);
    _tiles3_tex.connect_to<&example_tiling::_on_tex_loaded>(this);
    _tiles4_tex.connect_to<&example_tiling::_on_tex_loaded>(this);

    const auto [azimuth, elevation, orbit] =
      geo_coord.update(context().state().frame_duration().value()).get();
    camera.set_near(0.01F)
      .set_far(50.F)
      .set_orbit_min(1.02F)
      .set_orbit_max(2.2F)
      .set_fov(degrees_(55))
      .set_azimuth(azimuth)
      .set_elevation(elevation)
      .set_orbit_factor(orbit);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.connect_inputs()
      .add_ui_button("Change tile-set", {"GUI", "ChngTilset"})
      .connect_input(
        {"Example", "ChngTilset"},
        make_callable_ref<&example_tiling::change_tileset>(this))
      .map_inputs()
      .map_input(
        {"Example", "ChngTilset"}, {"Keyboard", "T"}, input_setup().trigger())
      .map_input(
        {"Example", "ChngTilset"},
        {"GUI", "ChngTilset"},
        input_setup().trigger())
      .switch_input_mapping();

    const auto& [gl, GL] = _video.gl_api();
    gl.clear_color(0.1F, 0.1F, 0.1F, 0.0F);
    gl.enable(GL.depth_test);
}
//------------------------------------------------------------------------------
void example_tiling::_on_loaded(const loaded_resource_base&) noexcept {
    if(_prog && _cube) {
        _prog->apply_input_bindings(_video, _cube);
        _prog->get_uniform_location(_video, "Camera") >> _camera_loc;
        _prog->get_uniform_location(_video, "TilesetTex") >> _tileset_tex_loc;
        oglplus::uniform_location tiling_tex_loc;
        _prog->get_uniform_location(_video, "TilingTex") >> tiling_tex_loc;
        _prog->set(_video, tiling_tex_loc, 0);
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_tiling::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL = _video.gl_api().constants();
    if(loaded.resource.is(_tiling_tex)) {
        loaded.parameter_i(GL.texture_min_filter, GL.nearest);
        loaded.parameter_i(GL.texture_mag_filter, GL.nearest);
        loaded.parameter_i(GL.texture_wrap_s, GL.repeat);
        loaded.parameter_i(GL.texture_wrap_t, GL.repeat);
    } else {
        loaded.parameter_i(GL.texture_min_filter, GL.linear_mipmap_linear);
        loaded.parameter_i(GL.texture_mag_filter, GL.linear);
        loaded.parameter_i(GL.texture_wrap_s, GL.repeat);
        loaded.parameter_i(GL.texture_wrap_t, GL.repeat);
        loaded.generate_mipmap();
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_tiling::change_tileset(const input& i) noexcept {
    if(!i) {
        tileset_tex_idx = (tileset_tex_idx + 1) % tileset_tex_units.size();
    }
}
//------------------------------------------------------------------------------
void example_tiling::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    if(_resources.are_loaded()) {
        if(state.user_idle_too_long()) {
            const auto [azimuth, elevation, orbit] =
              geo_coord.update(state.frame_duration().value()).get();
            if(geo_coord.is_done()) {
                switch(camera_status) {
                    case animation_status::relocate:
                        geo_coord.set({azimuth, elevation, 0.F}, 2.F);
                        camera_status = animation_status::zoom_in;
                        break;
                    case animation_status::zoom_in:
                        geo_coord.set({azimuth, elevation, 1.F}, 2.F);
                        camera_status = animation_status::zoom_out;
                        break;
                    case animation_status::zoom_out:
                        geo_coord.set(
                          {turns_(context().random_uniform_01()),
                           right_angles_(context().random_uniform_11()),
                           1.F},
                          3.F);
                        camera_status = animation_status::relocate;
                        break;
                }
            }
            camera.mark_changed()
              .set_azimuth(azimuth)
              .set_elevation(elevation)
              .set_orbit_factor(orbit);
        }

        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        _prog->set(
          _video, _tileset_tex_loc, tileset_tex_units[tileset_tex_idx]);
        _prog->set(_video, _camera_loc, camera.matrix(_video));

        _cube->draw(glapi);
    } else {
        _resources.load();
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_tiling::clean_up() noexcept {
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
               GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_tiling>(ec, vc)};
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
